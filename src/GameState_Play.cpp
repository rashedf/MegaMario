#include "GameState_Play.h"
#include "Common.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"

GameState_Play::GameState_Play(GameEngine & game, const std::string & levelPath)
    : GameState(game)
    , m_levelPath(levelPath)
{
    init(m_levelPath);
}

void GameState_Play::init(const std::string & levelPath)
{
    loadLevel(levelPath);

    spawnPlayer();

    // some sample entities
	/*
    auto brick = m_entityManager.addEntity("tile");
    brick->addComponent<CTransform>(Vec2(100, 400));
    brick->addComponent<CAnimation>(m_game.getAssets().getAnimation("Brick"), true);

    if (brick->getComponent<CAnimation>()->animation.getName() == "Brick")
    {
        std::cout << "This could be a good way of identifying if a tile is a brick!\n";
    }

    auto block = m_entityManager.addEntity("tile");
    block->addComponent<CTransform>(Vec2(200, 400));
    block->addComponent<CAnimation>(m_game.getAssets().getAnimation("Block"), true);
    // add a bounding box, this will now show up if we press the 'F' key
    block->addComponent<CBoundingBox>(m_game.getAssets().getAnimation("Block").getSize());

    auto question = m_entityManager.addEntity("tile");
    question->addComponent<CTransform>(Vec2(300, 400));
    question->addComponent<CAnimation>(m_game.getAssets().getAnimation("Question"), true);
	*/
}

void GameState_Play::loadLevel(const std::string & filename)
{
    // reset the entity manager every time we load a level
    m_entityManager = EntityManager();
    
    // TODO: read in the level file and add the appropriate entities
    //       use the PlayerConfig struct m_playerConfig to store player properties
    //       this struct is defined at the top of GameState_Play.h
	std::ifstream levelFile(filename);
	std::string token, animationName;
	int posx, posy;

	while (levelFile.good())
	{
		levelFile >> token;

		if (token == "Player")
		{
			levelFile >> m_playerConfig.X >> m_playerConfig.Y 
				      >> m_playerConfig.CX >> m_playerConfig.CY 
				      >> m_playerConfig.SPEED >> m_playerConfig.JUMP >> m_playerConfig.MAXSPEED >> m_playerConfig.GRAVITY 
				      >> m_playerConfig.WEAPON;
		}
		if (token == "Tile")
		{
			auto tile = m_entityManager.addEntity("tile");
			levelFile >> animationName >> posx >> posy;
			tile->addComponent<CAnimation>(m_game.getAssets().getAnimation(animationName), true);
			tile->addComponent<CTransform>(Vec2(posx, posy));
			tile->addComponent<CBoundingBox>(m_game.getAssets().getAnimation(animationName).getSize());
		}
		if (token == "Dec")
		{
			auto dec = m_entityManager.addEntity("dec");
			levelFile >> animationName >> posx >> posy;
			dec->addComponent<CAnimation>(m_game.getAssets().getAnimation(animationName), true);
			dec->addComponent<CTransform>(Vec2(posx, posy));
		}
	}
}

void GameState_Play::spawnPlayer()
{
    m_player = m_entityManager.addEntity("player");
    m_player->addComponent<CTransform>(Vec2(m_playerConfig.X, m_playerConfig.Y));
    m_player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CX, m_playerConfig.CY));
	m_player->addComponent<CState>("standing");
	m_player->addComponent<CGravity>(m_playerConfig.GRAVITY);
    m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("Stand"), true);
	m_player->addComponent<CInput>();
        
    // TODO: be sure to add the remaining components to the player
}

void GameState_Play::spawnBullet(std::shared_ptr<Entity> entity)
{
	auto bullet = m_entityManager.addEntity("bullet");
	bullet->addComponent<CTransform>(Vec2(entity->getComponent<CTransform>()->pos));
	bullet->getComponent<CTransform>()->scale = entity->getComponent<CTransform>()->scale;
	bullet->addComponent<CAnimation>(m_game.getAssets().getAnimation(m_playerConfig.WEAPON), true);
	bullet->addComponent<CBoundingBox>(bullet->getComponent<CAnimation>()->animation.getSize());
	bullet->addComponent<CLifeSpan>(1000);
}

void GameState_Play::update()
{
    m_entityManager.update();

    // TODO: implement pause functionality

    sMovement();
    sLifespan();
    sCollision();
    sAnimation();
    sUserInput();
    sRender();
}

void GameState_Play::sMovement()
{
	auto player_input = m_player->getComponent<CInput>();

	if (!player_input->left && !player_input->right && !player_input->up) {
		m_player->getComponent<CTransform>()->speed.x = 0;
		running = false;
	}

	if (m_player->getComponent<CInput>()->left)
	{
		m_player->getComponent<CTransform>()->speed.x = -m_playerConfig.SPEED;
		m_player->getComponent<CTransform>()->scale.x = -1;
		running = true;
	}
	else if (m_player->getComponent<CInput>()->right)
	{
		m_player->getComponent<CTransform>()->speed.x = m_playerConfig.SPEED;
		m_player->getComponent<CTransform>()->scale.x = 1;
		running = true;

	}
	if (m_player->getComponent<CInput>()->up && m_player->getComponent<CState>()->state != "jumping")
	{
		m_player->getComponent<CTransform>()->speed.y = m_playerConfig.JUMP;
	}

	m_player->getComponent<CTransform>()->speed.y += m_player->getComponent<CGravity>()->gravity;

	if (m_player->getComponent<CTransform>()->speed.y > m_playerConfig.MAXSPEED) {
		m_player->getComponent<CTransform>()->speed.y = m_playerConfig.MAXSPEED;
	}
	if (m_player->getComponent<CTransform>()->speed.y < -m_playerConfig.MAXSPEED) {
		m_player->getComponent<CTransform>()->speed.y = -m_playerConfig.MAXSPEED;
	}
	
	
	m_player->getComponent<CTransform>()->prevPos = m_player->getComponent<CTransform>()->pos;
	m_player->getComponent<CTransform>()->pos += m_player->getComponent<CTransform>()->speed;


	EntityVec bullets = m_entityManager.getEntities("bullet");
	for (std::size_t i = 0; i < bullets.size(); i++) {
		bullets[i]->getComponent<CTransform>()->pos.x += m_playerConfig.SPEED * 3 * bullets[i]->getComponent<CTransform>()->scale.x;
	}
    // TODO: Implement the maxmimum player speed in both X and Y directions
}

void GameState_Play::sLifespan()
{
    // TODO: Check lifespawn of entities that have them, and destroy them if they go over
	EntityVec entities = m_entityManager.getEntities();
	
	for (std::size_t i = 0; i < entities.size(); i++) {
		if (entities[i]->hasComponent<CLifeSpan>() && entities[i]->getComponent<CLifeSpan>()->clock.getElapsedTime().asMilliseconds() >= entities[i]->getComponent<CLifeSpan>()->lifespan) {
			entities[i]->destroy();
		}
	}
}

void GameState_Play::sCollision()
{

	EntityVec tiles = m_entityManager.getEntities("tile");
	EntityVec bullets = m_entityManager.getEntities("bullet");
	standing = false;

	// Check bullet-tile collisions
	for (std::size_t i = 0; i < tiles.size(); i++) {
		
		for (std::size_t j = 0; j < bullets.size(); j++) {
			auto overlap = Physics::GetOverlap(bullets[j], tiles[i]);
			if (overlap.x > 0 && overlap.y > 0) {
				bullets[j]->destroy();

				// Destroy the tile if it's a brick and set its animation to "Explosion"
				// Also, set the repeat to false so that it ends after one iteration
				if (tiles[i]->getComponent<CAnimation>()->animation.getName() == "Brick") {
					tiles[i]->getComponent<CAnimation>()->animation = m_game.getAssets().getAnimation("Explosion");
					tiles[i]->getComponent<CAnimation>()->repeat = false;
				}
			}
		}
		
		// Check Player-tile collsions

		auto current_overlap  = Physics::GetOverlap(tiles[i], m_player);
		auto previous_overlap = Physics::GetPreviousOverlap(tiles[i], m_player);

		// If there is a collision in the current frame, resolve it
		if (current_overlap.x > 0 && current_overlap.y > 0) {

			float delta_y = m_player->getComponent<CTransform>()->prevPos.y - m_player->getComponent<CTransform>()->pos.y;
			float delta_x = m_player->getComponent<CTransform>()->prevPos.x - m_player->getComponent<CTransform>()->pos.x;
			
			// If player came from above/below the tile
			if (previous_overlap.x > 0) {
				m_player->getComponent<CTransform>()->pos.y += current_overlap.y * ((delta_y > 0) - (delta_y < 0));
				// If palyer came from above the tile
				if (((delta_y > 0) - (delta_y < 0)) > 0) {
					m_player->getComponent<CTransform>()->speed.y = 0;
					standing = true;
				}
				// If player came from below the tile
				else if (((delta_y > 0) - (delta_y < 0)) < 0) {
					m_player->getComponent<CTransform>()->speed.y = 0;
					// Set Brick tiles to Explosion tiles, and set repeat to false
					if (tiles[i]->getComponent<CAnimation>()->animation.getName() == "Brick") {
						tiles[i]->getComponent<CAnimation>()->animation = m_game.getAssets().getAnimation("Explosion");
						tiles[i]->getComponent<CAnimation>()->repeat = false;
					}
					// Set Question tiles to Question2 tiles, leave repeat as true
					// Create a new entity above the Question tile, give it a Coin animation and set repeat to true, give it a lifespan component
					else if (tiles[i]->getComponent<CAnimation>()->animation.getName() == "Question") {
						tiles[i]->getComponent<CAnimation>()->animation = m_game.getAssets().getAnimation("Question2");
						auto coin = m_entityManager.addEntity("dec");
						coin->addComponent<CAnimation>(m_game.getAssets().getAnimation("Coin"), true);
						coin->addComponent<CTransform>(tiles[i]->getComponent<CTransform>()->pos + Vec2(0, tiles[i]->getComponent<CBoundingBox>()->size.y));
						coin->addComponent<CLifeSpan>(1000);
					}
				}
			}
			// If player came from left/right of the tile
			else if (previous_overlap.y > 0) {
				m_player->getComponent<CTransform>()->pos.x += current_overlap.x * ((delta_x > 0) - (delta_x < 0));
			}
			// If player came from a position diagonal to the tile
			else { 
				if (((delta_y > 0) - (delta_y < 0)) < 0) {
					m_player->getComponent<CTransform>()->speed.y = 0;
				}
			}
		}
		
		
	}

	if		(!standing) { m_player->getComponent<CState>()->state = "jumping"; }
	else if (!running)	{ m_player->getComponent<CState>()->state = "standing"; }
	else				{ m_player->getComponent<CState>()->state = "running"; }

	if (m_player->getComponent<CTransform>()->pos.y < 0) { 
		m_player->destroy(); 
		spawnPlayer(); 
	}
	if (m_player->getComponent<CTransform>()->pos.x < m_player->getComponent<CBoundingBox>()->halfSize.x) {
		m_player->getComponent<CTransform>()->pos.x = m_player->getComponent<CBoundingBox>()->halfSize.x;
	}
}

void GameState_Play::sUserInput()
{
    // TODO: implement the rest of the player input

    sf::Event event;
    while (m_game.window().pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            m_game.quit();
        }

        if (event.type == sf::Event::KeyPressed)
        {
            switch (event.key.code)
            {
                case sf::Keyboard::Escape:  { m_game.popState(); break; }
                case sf::Keyboard::Z:       { init(m_levelPath); break; }
                case sf::Keyboard::R:       { m_drawTextures = !m_drawTextures; break; }
                case sf::Keyboard::F:       { m_drawCollision = !m_drawCollision; break; }
                case sf::Keyboard::P:       { setPaused(!m_paused); break; }
				
				case sf::Keyboard::A:		{ m_player->getComponent<CInput>()->left = true; break; }
				case sf::Keyboard::D:		{ m_player->getComponent<CInput>()->right = true; break; }
				case sf::Keyboard::W:		{ m_player->getComponent<CInput>()->up = true; break; }
				case sf::Keyboard::S:		{ m_player->getComponent<CInput>()->down = true; break; }
				case sf::Keyboard::Space:	{ spawnBullet(m_player); break; }
            }
        }

        if (event.type == sf::Event::KeyReleased)
        {
            switch (event.key.code)
            {
                case sf::Keyboard::Escape: { break; }

				case sf::Keyboard::A: { m_player->getComponent<CInput>()->left = false; break; }
				case sf::Keyboard::D: { m_player->getComponent<CInput>()->right = false; break; }
				case sf::Keyboard::W: { m_player->getComponent<CInput>()->up = false; break; }
				case sf::Keyboard::S: { m_player->getComponent<CInput>()->down = false; break; }
            }
        }
    }

}

void GameState_Play::sAnimation()
{

	if (m_player->getComponent<CState>()->state == "standing") {
		m_player->getComponent<CAnimation>()->animation = m_game.getAssets().getAnimation("Stand");
	}
	else if (m_player->getComponent<CState>()->state == "jumping") {
		m_player->getComponent<CAnimation>()->animation = m_game.getAssets().getAnimation("Air");
	}
	else if (m_player->getComponent<CState>()->state == "running" && m_player->getComponent<CAnimation>()->animation.getName() != "Run") {
		m_player->getComponent<CAnimation>()->animation = m_game.getAssets().getAnimation("Run");
	}

	auto entities = m_entityManager.getEntities();

	for (std::size_t i = 0; i < entities.size(); i++) {
		if (entities[i]->hasComponent<CAnimation>()) {
			entities[i]->getComponent<CAnimation>()->animation.update();
			if (!entities[i]->getComponent<CAnimation>()->repeat && entities[i]->getComponent<CAnimation>()->animation.hasEnded()) {
				entities[i]->destroy();
			}
		}
	}
    // TODO: for each entity with an animation, call entity->getComponent<CAnimation>()->animation.update()
    //       if the animation is not repeated, and it has ended, destroy the entity
	// check if it's a non repeating animation
	// if it is !repeating check hasEnded() and destroy the entity 
}

// this function has been completed for you
void GameState_Play::sRender()
{
    // color the background darker so you know that the game is paused
    if (!m_paused)  { m_game.window().clear(sf::Color(100, 100, 255)); }
    else            { m_game.window().clear(sf::Color(50, 50, 150)); }

    // set the viewport of the window to be centered on the player if it's far enough right
    auto pPos = m_player->getComponent<CTransform>()->pos;
    float windowCenterX = fmax(m_game.window().getSize().x / 2.0f, pPos.x);
    sf::View view = m_game.window().getView();
    view.setCenter(windowCenterX, m_game.window().getSize().y - view.getCenter().y);
    m_game.window().setView(view);
        
    // draw all Entity textures / animations
    if (m_drawTextures)
    {
        for (auto e : m_entityManager.getEntities())
        {
            auto transform = e->getComponent<CTransform>();

            if (e->hasComponent<CAnimation>())
            {
                auto animation = e->getComponent<CAnimation>()->animation;
                animation.getSprite().setRotation(transform->angle);
                animation.getSprite().setPosition(transform->pos.x, m_game.window().getSize().y - transform->pos.y);
                animation.getSprite().setScale(transform->scale.x, transform->scale.y);
                m_game.window().draw(animation.getSprite());
            }
        }
    }

    // draw all Entity collision bounding boxes with a rectangleshape
    if (m_drawCollision)
    {
        for (auto e : m_entityManager.getEntities())
        {
            if (e->hasComponent<CBoundingBox>())
            {
                auto box = e->getComponent<CBoundingBox>();
                auto transform = e->getComponent<CTransform>();
                sf::RectangleShape rect;
                rect.setSize(sf::Vector2f(box->size.x-1, box->size.y-1));
                rect.setOrigin(sf::Vector2f(box->halfSize.x, box->halfSize.y));
                rect.setPosition(transform->pos.x, m_game.window().getSize().y - transform->pos.y);
                rect.setFillColor(sf::Color(0, 0, 0, 0));
                rect.setOutlineColor(sf::Color(255, 255, 255, 255));
                rect.setOutlineThickness(1);
                m_game.window().draw(rect);
            }
        }
    }

    m_game.window().display();
}