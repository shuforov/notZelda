#include <cmath>
#include <utility>

#include "../include/Animation.h"


Animation::Animation() = default;

Animation::Animation(const std::string &name, const sf::Texture &t)
        : Animation(name, t, 1, 0) {}

Animation::Animation(std::string name, const sf::Texture &t, size_t frameCount, size_t speed)
        : m_name(std::move(name)), m_sprite(t), m_frameCount(frameCount), m_currentFrame(0), m_speed(speed) {
    m_size = vec2((float) t.getSize().x / float(frameCount), (float) t.getSize().y);
    m_sprite.setOrigin(m_size.x / 2.0f, m_size.y / 2.0f);
    m_sprite.setTextureRect(sf::IntRect(std::floor(float(m_currentFrame) * m_size.x), 0, int(m_size.x), int(m_size.y)));
}

// updates the animation to show the next frame, depending on its speed
// animation loops when it reaches the end
void Animation::update(bool flipped) {
  // add the speed variable to the current frame
  if (m_speed == 0 || m_frameCount == 0) {
    return;
  }

  if (hasEnded()) {
    m_currentFrame = 0;
  }
  m_currentFrame++;
  int animationFrame = (m_currentFrame / m_speed) % m_frameCount;
  sf::IntRect rectangle(animationFrame * m_size.x, 0, m_size.x, m_size.y);
  m_sprite.setTextureRect(rectangle);
  setFlipped(flipped);
}

bool Animation::hasEnded() const {
  // detect when animation has ended (last frame waw played) and return true
  // if (m_speed == 0) return true;
  return (m_currentFrame / m_speed) >= m_frameCount;
}

const vec2 &Animation::getSize() const {
    return m_size;
}

const std::string &Animation::getName() const {
    return m_name;
}

const sf::Sprite &Animation::getSprite() const {
    return m_sprite;
}

sf::Sprite &Animation::getSprite() {
    return m_sprite;
}

void Animation::setFlipped(bool flipped) {
  sf::IntRect rect = m_sprite.getTextureRect();

  if (flipped && rect.width > 0) {
    rect.left = rect.left + rect.width;
    rect.width = -rect.width;
  } else if (!flipped && rect.width < 0) {
    rect.left = rect.left + rect.width; // since width is negative
    rect.width = -rect.width;
  }
  m_sprite.setTextureRect(rect);
}
