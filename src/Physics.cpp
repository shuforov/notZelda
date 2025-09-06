#include "../include/Physics.h"


vec2 Physics::GetOverlap(const std::shared_ptr<Entity> &a, const std::shared_ptr<Entity> &b) {
    // return the overlap rectangle size of the bounding boxes of entity a and b
    vec2 posA = a->get<CTransform>().pos;
    vec2 posB = b->get<CTransform>().pos;
    vec2 halfSizeA = a->get<CBoundingBox>().halfSize;
    vec2 halfSizeB = b->get<CBoundingBox>().halfSize;

    vec2 delta{std::abs(posA.x - posB.x), std::abs(posA.y - posB.y)};
    return halfSizeA + halfSizeB - delta;
}

vec2 Physics::GetPreviousOverlap(const std::shared_ptr<Entity> &a, const std::shared_ptr<Entity> &b) {
    // return the previous overlap rectangle size of the bounding boxes of entity a and b
    // previous overlap uses the entity's previous position
    vec2 posA = a->get<CTransform>().prevPos;
    vec2 posB = b->get<CTransform>().prevPos;
    vec2 halfSizeA = a->get<CBoundingBox>().halfSize;
    vec2 halfSizeB = b->get<CBoundingBox>().halfSize;

    vec2 delta{std::abs(posA.x - posB.x), std::abs(posA.y - posB.y)};
    return halfSizeA + halfSizeB - delta;
}

bool Physics::IsInside(const vec2 &pos, const std::shared_ptr<Entity> &entity) {
    vec2 eSize = entity->get<CAnimation>().animation.getSize();
    vec2 ePos = entity->get<CTransform>().pos;

    if (pos.x > ePos.x - eSize.x / 2 &&
        pos.x < ePos.x + eSize.x / 2 &&
        pos.y > ePos.y - eSize.y / 2 &&
        pos.y < ePos.y + eSize.y / 2) {
        // std::cout << entity->getComponent<CAnimation>().animation.name() << "\n";
        return true;
    }

    return false;
}

Intersect Physics::LineIntersect(const vec2 &a, const vec2 &b, const vec2 &c,
                                 const vec2 &d) {
  // Example: a is start point of ray and b is end ray point (to player) and c
  // is start line intersect and d is end of line intersect
  // LineIntersect(vec2(2,3), vec2(9,8), vec2(2,9), vec2(9, 4)); This should
  // return true(1) and value is somwhere in vec(6.2,6)
  vec2 r = (b - a);
  vec2 s = (d - c);
  float rxs = r.x * s.y - r.y * s.x; // it is cross product
  vec2 cma = c - a;
  float t = (cma.x * s.y - cma.y * s.x) / rxs;
  float u = (cma.x * r.y - cma.y * r.x) / rxs;
  if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
    return {true, vec2(a.x + t * r.x, a.y + t * r.y)};
  }
  return {false, vec2(0, 0)};
}

bool Physics::EntityIntersect(const vec2 &a, const vec2 &b, const std::shared_ptr<Entity> &entity) {
    // Student TODO
    return false;
}
