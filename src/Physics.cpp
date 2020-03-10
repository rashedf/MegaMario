#include "Physics.h"
#include "Components.h"
#include <cmath>

Vec2 Physics::GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
	auto a_pos = a->getComponent<CTransform>()->pos;
	auto b_pos = b->getComponent<CTransform>()->pos;

	float delta_x = abs(a_pos.x - b_pos.x);
	float delta_y = abs(a_pos.y - b_pos.y);

	float overlap_x = a->getComponent<CBoundingBox>()->halfSize.x + b->getComponent<CBoundingBox>()->halfSize.x - delta_x;
	float overlap_y = a->getComponent<CBoundingBox>()->halfSize.y + b->getComponent<CBoundingBox>()->halfSize.y - delta_y;

    return Vec2(overlap_x, overlap_y);
}

Vec2 Physics::GetPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
	auto a_pos = a->getComponent<CTransform>()->pos;
	auto b_pos = b->getComponent<CTransform>()->prevPos;

	float delta_x = abs(a_pos.x - b_pos.x);
	float delta_y = abs(a_pos.y - b_pos.y);

	float overlap_x = a->getComponent<CBoundingBox>()->halfSize.x + b->getComponent<CBoundingBox>()->halfSize.x - delta_x;
	float overlap_y = a->getComponent<CBoundingBox>()->halfSize.y + b->getComponent<CBoundingBox>()->halfSize.y - delta_y;

	return Vec2(overlap_x, overlap_y);
}
