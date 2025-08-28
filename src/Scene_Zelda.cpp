#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/System/Vector2.hpp"
#include "SFML/Window/Event.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "../libs/imgui/imgui-SFML.h"
#include "../libs/imgui/imgui.h"

#include "../include/Components.h"
#include "../include/GameEngine.h"
#include "../include/Scene_Zelda.h"
#include "../include/Scene_Menu.h"

Scene_Zelda::Scene_Zelda(GameEngine *game, std::string &levelPath)
    : Scene(game), m_levelPath(levelPath) {
  init(m_levelPath);
}

void Scene_Zelda::init(const std::string &levelPath) {
  loadLevel(levelPath);

  m_gridText.setCharacterSize(12);
  m_gridText.setFont(m_game->assets().getFont("Tech"));

  registerAction(sf::Keyboard::Escape, "QUIT");
  registerAction(sf::Keyboard::P, "PAUSE");
  registerAction(sf::Keyboard::Y, "TOGGLE_FOLLOW"); // toggle follow camera
  registerAction(sf::Keyboard::T,
                 "TOGGLE_TEXTURE"); // toggle drawing (T)extures
  registerAction(sf::Keyboard::C,
                 "TOGGLE_COLLISION"); // toggle drawing (C)ollision Box
  registerAction(sf::Keyboard::G, "TOGGLE_GRID"); // toggle drawing (G)rid
  registerAction(sf::Keyboard::W, "UP");
  registerAction(sf::Keyboard::A, "LEFT");
  registerAction(sf::Keyboard::D, "RIGHT");
  registerAction(sf::Keyboard::S, "DOWN");
  registerAction(sf::Keyboard::Space, "ATTACK");

  // TODO: Register the actions required to play the game
}

void Scene_Zelda::loadLevel(const std::string &fileName) {
  m_entityManager = EntityManager();

  // TODO:
  // Load the level file and put all entities in the manager
  // Use the getPosition() function below to convert room-tile coords to game
  // world coords
  // Reading data in level file here
  std::ifstream fileInput(fileName);
  if (!fileInput.is_open()) {
    std::cerr << "Could not open config file: " << fileName << std::endl;
    exit(1);
  }
  std::string configName;
  std::string entityName;
  vec2 roomPos;
  vec2 gridPos;
  int tileMovement;
  int tileBlockMovement;
  int maxHealth;
  int damage;
  int blockMovementNpc;
  int blockVisionNpc;
  while (fileInput >> configName) {
    if (configName == "Player") {
      fileInput >> m_playerConfig.X >> m_playerConfig.Y >> m_playerConfig.CX >>
          m_playerConfig.CY >> m_playerConfig.SPEED >> m_playerConfig.HEALTH;
    } else if (configName == "Tile") {
      fileInput >> entityName >> roomPos.x >> roomPos.y >> gridPos.x >>
          gridPos.y >> tileMovement >> tileBlockMovement;
      auto tileNode = m_entityManager.addEntity("Tile");
      tileNode->add<CAnimation>(m_game->assets().getAnimation(entityName),
                                true);
      vec2 tilePosition = gridToMidPixel(gridPos.x, gridPos.y, tileNode);
      Animation tileAnimation = tileNode->get<CAnimation>().animation;
      tileNode->add<CTransform>(tilePosition);
      tileNode->add<CBoundingBox>(tileAnimation.getSize(),
                                  tileAnimation.getSize(), tileMovement,
                                  tileBlockMovement);
    } else if (configName == "NPC") {
      fileInput >> entityName >> roomPos.x >> roomPos.y >> gridPos.x >>
	gridPos.y >> blockMovementNpc >> blockVisionNpc >> maxHealth >> damage;
      auto NPCNode = m_entityManager.addEntity("NPC");
      NPCNode->add<CAnimation>(m_game->assets().getAnimation(entityName),
                                true);
      vec2 NPCPosition = gridToMidPixel(gridPos.x, gridPos.y, NPCNode);
      Animation tileAnimation = NPCNode->get<CAnimation>().animation;
      NPCNode->add<CTransform>(NPCPosition);
      NPCNode->add<CBoundingBox>(tileAnimation.getSize(),
                                  tileAnimation.getSize(), tileMovement,
                                  tileBlockMovement);
      NPCNode->add<CHealth>(maxHealth, maxHealth);
      NPCNode->add<CInvincibility>(0); // 0 is how mouch at current moment npc can be invicnible
    }
  }
  spawnPlayer();
}

vec2 Scene_Zelda::getPosition(int rx, int ry, int tx, int ty) const {
  // TODO:
  // Implement this function, which takes in the room (rx, ry)
  // as well as the tile (tx, ty), and return the vec2 game world
  // position of the center of the entity

  return vec2(0, 0);
}

void Scene_Zelda::spawnPlayer() {
  auto p = m_entityManager.addEntity("player");
  m_player = p;
  p->add<CTransform>(vec2(m_playerConfig.X, m_playerConfig.Y));
  p->get<CTransform>().prevPos = p->get<CTransform>().pos;
  p->add<CAnimation>(m_game->assets().getAnimation("LinkStandDown"), true);
  p->add<CBoundingBox>(vec2(m_playerConfig.X, m_playerConfig.Y), vec2(48, 48), true, false);
  p->add<CDraggable>(); // just to test draggable
  p->add<CHealth>(m_playerConfig.HEALTH, 3);

  // TODO:
  // Implement this function so that it uses the parameters input from the level
  // file Those parameters should be stored in the m_playerConfig variable
}

void Scene_Zelda::spawnSword(std::shared_ptr<Entity> entity) {
  // TODO:
  // Implement the spawning of the sword, which:
  // - should be given the appropriate lifespan
  // - should spawn at the appropriate location based on player's facing
  // direction
  // - be given a damage value of 1
  // - should play the "Slash" sound
  const int SPRITE_SIZE = 64;
  auto swordNode = m_entityManager.addEntity("sword");
  auto playerPosition = entity->get<CTransform>().pos;
  swordNode->add<CTransform>(playerPosition);
  swordNode->add<CBoundingBox>(vec2(SPRITE_SIZE, SPRITE_SIZE),
                               vec2(SPRITE_SIZE, SPRITE_SIZE), false, false);
  swordNode->add<CLifespan>(10, 0);
  auto &swordTransform = swordNode->get<CTransform>();
  auto &playerAnimation = m_player->get<CAnimation>().animation;
  swordTransform.prevPos = swordTransform.pos;
  if (m_playerLookAt == "DOWN") {
    swordTransform.pos.y += SPRITE_SIZE;
    if (playerAnimation.getName() != "LinkAtkDown") {
      m_player->add<CAnimation>(m_game->assets().getAnimation("LinkAtkDown"),
                                false);
    }
    playerAnimation.setFlipped(false);
  } else if (m_playerLookAt == "UP") {
    swordTransform.pos.y -= SPRITE_SIZE;
    if (playerAnimation.getName() != "LinkAtkUp") {
      m_player->add<CAnimation>(m_game->assets().getAnimation("LinkAtkUp"),
                                false);
    }
    playerAnimation.setFlipped(false);
  } else if (m_playerLookAt == "RIGHT") {
    swordTransform.pos.x += SPRITE_SIZE;
    if (playerAnimation.getName() != "LinkAtkRight") {
      m_player->add<CAnimation>(m_game->assets().getAnimation("LinkAtkRight"),
                                false);
    }
    playerAnimation.setFlipped(false);
  } else if (m_playerLookAt == "LEFT") {
    playerAnimation.setFlipped(false);
    swordTransform.pos.x -= SPRITE_SIZE;
    if (playerAnimation.getName() != "LinkAtkRight") {
      m_player->add<CAnimation>(m_game->assets().getAnimation("LinkAtkRight"),
                                false);
    }
    playerAnimation.setFlipped(true);
  }
  m_isAttack = true;
}

void Scene_Zelda::update() {
  m_entityManager.update();

  // TODO:
  // Implement pause functionality

  sDrag();
  sAI();
  sMovement();
  sStatus();
  sCollision();
  sAnimation();
  sCamera();
  // sGUI();
  sRender();

  m_currentFrame++;
}

void Scene_Zelda::sMovement() {
  // TODO:
  // Implement all player movement functionality here based on
  // the player's input component variables
  auto &playerTransform = m_player->get<CTransform>();
  auto &playerInputs = m_player->get<CInput>();
  auto &playerAnimation = m_player->get<CAnimation>().animation;
  playerTransform.velocity = vec2(0, 0);
  if (!m_isAttack) {
    if (playerInputs.up) {
      playerTransform.velocity.y = -1;
      if (playerAnimation.getName() != "LinkMoveUp") {
        m_player->add<CAnimation>(m_game->assets().getAnimation("LinkMoveUp"),
                                  true);
      }
      m_animationIsFlipped = false;
      playerAnimation.setFlipped(false);
      m_playerLookAt = "UP";
      m_isMoving = true;
    } else if (playerInputs.down) {
      playerTransform.velocity.y = 1;
      if (playerAnimation.getName() != "LinkMoveDown") {
        m_player->add<CAnimation>(m_game->assets().getAnimation("LinkMoveDown"),
                                  true);
      }
      m_animationIsFlipped = false;
      playerAnimation.setFlipped(false);
      m_playerLookAt = "DOWN";
      m_isMoving = true;
    } else if (playerInputs.left) {
      playerTransform.velocity.x = -1;
      if (playerAnimation.getName() != "LinkMoveRight") {
        m_player->add<CAnimation>(
            m_game->assets().getAnimation("LinkMoveRight"), true);
      }
      m_playerLookAt = "LEFT";
      playerAnimation.setFlipped(true);
      m_animationIsFlipped = true;
      m_isMoving = true;
    } else if (playerInputs.right) {
      playerTransform.velocity.x = 1;
      if (playerAnimation.getName() != "LinkMoveRight") {
        m_player->add<CAnimation>(
            m_game->assets().getAnimation("LinkMoveRight"), true);
      }
      m_playerLookAt = "RIGHT";
      playerAnimation.setFlipped(false);
      m_animationIsFlipped = false;
      m_isMoving = true;
    } else {
      m_isMoving = false;
    }
  }
  playerTransform.prevPos = playerTransform.pos;
  playerTransform.pos += playerTransform.velocity * m_playerConfig.SPEED;

  // Sword movement
  for (auto &entity : m_entityManager.getEntities("sword")) {
    if (m_playerLookAt == "DOWN") {
      entity->get<CTransform>().pos = vec2(
          playerTransform.pos.x,
          playerTransform.pos.y + 32); // 32 is half size of sprite size of 64
    } else if (m_playerLookAt == "UP") {
      entity->get<CTransform>().pos = vec2(
          playerTransform.pos.x,
          playerTransform.pos.y - 32); // 32 is half size of sprite size of 64
    } else if (m_playerLookAt == "RIGHT") {
      entity->get<CTransform>().pos =
          vec2(playerTransform.pos.x + 32,
               playerTransform.pos.y); // 32 is half size of sprite size of 64
    } else if (m_playerLookAt == "LEFT") {
      entity->get<CTransform>().pos =
          vec2(playerTransform.pos.x - 32,
               playerTransform.pos.y); // 32 is half size of sprite size of 64
    }
  }
}

void Scene_Zelda::sGUI() {
  ImGui::Begin("Scene Properties");

  if (ImGui::BeginTabBar("MyTabBar")) {

    if (ImGui::BeginTabItem("Debug")) {
      ImGui::Checkbox("Draw Grid (G)", &m_drawGrid);
      ImGui::Checkbox("Draw Textures (T)", &m_drawTextures);
      ImGui::Checkbox("Draw Debug (C)", &m_drawCollision);
      ImGui::Checkbox("Follow Cam (Y)", &m_follow);

      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Animations")) {
      // TODO:
      ImGui::Text("Do this");

      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Entity Manager")) {
      // TODO:
      ImGui::Text("Do this too");

      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }
  ImGui::End();
}

void Scene_Zelda::sDoAction(const Action &action) {
  // TODO:
  // Implement all actions described for the game here
  // Only the setting of the player's input component variables should be set
  // here Do minimal logic in this function

  if (action.name() == "MOUSE_MOVE") {
    m_mousePos = action.pos();
  }

  if (action.type() == "START") {
    if (action.name() == "PAUSE") {
      setPaused(!m_paused);
    } else if (action.name() == "QUIT") {
      onEnd();
    } else if (action.name() == "TOGGLE_FOLLOW") {
      m_follow = !m_follow;
    } else if (action.name() == "TOGGLE_TEXTURE") {
      m_drawTextures = !m_drawTextures;
    } else if (action.name() == "TOGGLE_COLLISION") {
      m_drawCollision = !m_drawCollision;
    } else if (action.name() == "TOGGLE_GRID") {
      m_drawGrid = !m_drawGrid;
    } else if (action.name() == "LEFT_CLICK") {
      // detect the picking up of entities
      vec2 wPos = windowToWorld(m_mousePos);
      for (const auto &entity : m_entityManager.getEntities()) {
        if (!entity->has<CDraggable>()) {
          continue;
        }

        if (Physics::IsInside(wPos, entity)) {
          auto &dragging = entity->get<CDraggable>().dragging;
          dragging = !dragging;
        }
      }
    } else if (action.name() == "UP") {
      m_player->get<CInput>().up = true;
    } else if (action.name() == "LEFT") {
      m_player->get<CInput>().left = true;
    } else if (action.name() == "RIGHT") {
      m_player->get<CInput>().right = true;
    } else if (action.name() == "DOWN") {
      m_player->get<CInput>().down = true;
    } else if (action.name() == "ATTACK") {
      m_player->get<CInput>().attack = true;
      spawnSword(m_player);
    }
  } else if (action.type() == "END") {
    if (action.name() == "UP") {
      m_player->get<CInput>().up = false;
    } else if (action.name() == "LEFT") {
      m_player->get<CInput>().left = false;
    } else if (action.name() == "RIGHT") {
      m_player->get<CInput>().right = false;
    } else if (action.name() == "DOWN") {
      m_player->get<CInput>().down = false;
    } else if (action.name() == "ATTACK") {
      m_player->get<CInput>().attack = false;
    }
  }
}

void Scene_Zelda::sAI() {
  // TODO: Implement enemy AI
  // Implement Follow behavior
  // Implement Patrol behavior
}

void Scene_Zelda::sStatus() {
  // TODO:
  // Implement Lifespan here
  // Implement Invincibility Frames here
  for (auto &entityNode : m_entityManager.getEntities("sword")) {
    auto &lifeData = entityNode->get<CLifespan>();
    if (lifeData.lifespan == lifeData.frameCreated) {
      swapPlayerAnimationToStand();
      m_isAttack = false;
      entityNode->destroy();
    } else {
      lifeData.frameCreated++;
    }
  }
}

void Scene_Zelda::sCollision() {
  // TODO:
  // Implement entity - tile collisions
  // Implement player - enemy collisions with appropriate damage calculations
  // Implement sword - NPC collisions
  // Implement entity - heart collisions and life gain logic
  // Implement black tile collisions / 'teleporting'
  // You may want to use helper functions for these behaviors or this function
  // will get long
  vec2 &playerPosition = m_player->get<CTransform>().pos;
  vec2 &playerPrevPosition = m_player->get<CTransform>().prevPos;
  for (auto &entityNode : m_entityManager.getEntities("Tile")) {
    vec2 overlap = m_worldPhysics.GetOverlap(m_player, entityNode);
    vec2 overlapPrev = m_worldPhysics.GetPreviousOverlap(m_player, entityNode);
    bool entityBBBlockMove = entityNode->get<CBoundingBox>().blockMove;
    if (overlap != vec2(0, 0)) {
      if (overlap.x > 0 && overlap.y > 0) {
	if (entityBBBlockMove) {
          if (overlapPrev.x < overlap.x) {
            playerPosition.x = playerPrevPosition.x;
          }
          if (overlapPrev.y < overlap.y) {
            playerPosition.y = playerPrevPosition.y;
          }
        }
      }
    }
  }

  for (auto &entityNode : m_entityManager.getEntities("NPC")) {
    int &entityNodeInvinc = entityNode->get<CInvincibility>().iframes;
    for (auto &swordNode : m_entityManager.getEntities("sword")) {
      vec2 overlap = m_worldPhysics.GetOverlap(swordNode, entityNode);
      vec2 overlapPrev =
          m_worldPhysics.GetPreviousOverlap(swordNode, entityNode);
      bool entityBBBlockMove = entityNode->get<CBoundingBox>().blockMove;
      vec2 swordPosition = swordNode->get<CTransform>().pos;
      vec2 swordPrevPosition = swordNode->get<CTransform>().prevPos;
      if (overlap != vec2(0, 0)) {
        if (overlap.x > 0 && overlap.y > 0) {
          if (entityNodeInvinc == 0) {
            entityNode->get<CHealth>().current -= 1;
            entityNodeInvinc = 30; // 30 is count of frames
          }
        }
      }
    }
    // counting down invincibility timer till it will be 0 and npc can be attacked by player agen
    if (entityNodeInvinc > 0) {
      entityNodeInvinc -= 1;
    }
  }
}

void Scene_Zelda::sAnimation() {
  // TODO:
  // Implement player facing direction animation
  // Implement sword animation based on player facing
  // The sword should move if the player changes direction mid swing
  // Implement destruction of entities with non-repeating finished animations
  auto &playerAnimation = m_player->get<CAnimation>().animation;
  playerAnimation.update(m_animationIsFlipped);
  if (playerAnimation.getName() == "LinkAtkDown") {
    m_isAttack = true;
  } else if (playerAnimation.getName() == "LinkAtkUp") {
    m_isAttack = true;
  } else if (playerAnimation.getName() == "LinkAtkRight") {
    m_isAttack = true;
  } else {
    if (!m_isMoving) {
      swapPlayerAnimationToStand();
    }
  }
  for (auto &entityNode : m_entityManager.getEntities("NPC")) {
    entityNode->get<CAnimation>().animation.update(false);
  }
}

void Scene_Zelda::swapPlayerAnimationToStand() {
  auto &playerAnimation = m_player->get<CAnimation>().animation;
  if (m_playerLookAt == "UP") {
    if (playerAnimation.getName() != "LinkStandUp") {
      m_player->add<CAnimation>(m_game->assets().getAnimation("LinkStandUp"),
                                true);
      playerAnimation.setFlipped(false);
      m_animationIsFlipped = false;
    }
  } else if (m_playerLookAt == "DOWN") {
    if (playerAnimation.getName() != "LinkStandDown") {
      m_player->add<CAnimation>(m_game->assets().getAnimation("LinkStandDown"),
                                true);
      playerAnimation.setFlipped(false);
      m_animationIsFlipped = false;
    }
  } else if (m_playerLookAt == "RIGHT") {
    if (playerAnimation.getName() != "LinkStandRight") {
      m_player->add<CAnimation>(m_game->assets().getAnimation("LinkStandRight"),
                                true);
      playerAnimation.setFlipped(false);
      m_animationIsFlipped = false;
    }
  } else if (m_playerLookAt == "LEFT") {
    if (playerAnimation.getName() != "LinkStandRight") {
      m_player->add<CAnimation>(m_game->assets().getAnimation("LinkStandRight"),
                                true);
      playerAnimation.setFlipped(true);
      m_animationIsFlipped = true;
    }
  }
}

void Scene_Zelda::sCamera() {
  // TODO:
  // Implement camera view logic

  // get the current view, which we will modify in the if-statement below
  sf::View view = m_game->window().getView();
  sf::Vector2f viewCenter = view.getCenter();
  auto windowSize = m_game->window().getSize();
  if (m_follow) {
    // calculate view for player follow camera
  } else {
    if (playerMovingToRoomDiraction() == "UP") {
      // Switch camera to up room
      vec2 upRoomCenter = vec2(viewCenter.x, viewCenter.y - windowSize.y);
      view.setCenter(upRoomCenter.x, upRoomCenter.y);
    } else if (playerMovingToRoomDiraction() == "DOWN") {
      // Switch camera to down room
      vec2 downRoomCenter = vec2(viewCenter.x, viewCenter.y + windowSize.y);
      view.setCenter(downRoomCenter.x, downRoomCenter.y);
    } else if (playerMovingToRoomDiraction() == "LEFT") {
      // Switch camera to left room
      vec2 leftRoomCenter = vec2(viewCenter.x - windowSize.x, viewCenter.y);
      view.setCenter(leftRoomCenter.x, leftRoomCenter.y);
    } else if (playerMovingToRoomDiraction() == "RIGHT") {
      // Switch camera to right room
      vec2 rightRoomCenter = vec2(viewCenter.x + windowSize.x, viewCenter.y);
      std::cout << "view current position: " << view.getCenter().x << ", " << view.getCenter().y << std::endl;
      view.setCenter(rightRoomCenter.x, rightRoomCenter.y);
      std::cout << "right room center: " << rightRoomCenter.x << ", "
                << rightRoomCenter.y << std::endl;
      std::cout << "view updated position: " << view.getCenter().x << ", " << view.getCenter().y << std::endl;
    }
  }

  // then set the window view
  m_game->window().setView(view);
}

std::string Scene_Zelda::playerMovingToRoomDiraction() {
  vec2 playerPosition = m_player->get<CTransform>().pos;
  auto windowSize = m_game->window().getSize();
  sf::View view = m_game->window().getView();
  sf::Vector2f viewCenter = view.getCenter();
  sf::Vector2f size = view.getSize();
  // viewPosition is top left coordinates of view port
  vec2 viewPosition =
      vec2((viewCenter.x - size.x / 2.f), (viewCenter.y - size.y / 2.f));
  if (playerPosition.x > viewPosition.x + windowSize.x) {
    // Player moving to right room
    return "RIGHT";
  } else if (playerPosition.x < viewPosition.x) {
    // Player moving to left room
    return "LEFT";
  } else if (playerPosition.y > viewPosition.y + windowSize.y) {
    // Player moving down room
    return "DOWN";
  } else if (playerPosition.y < viewPosition.y) {
    // Player moving up room
    return "UP";
  } else {
    // Player in range of room
    return "IN_ROOM";
  }
}

void Scene_Zelda::onEnd() {
  // TODO
  // When the scene ends, change back to the MENU scene
  // Stop the music
  // Play the menu music
  // Use m_game->changeScene(correct params);
  m_game->changeScene("MENU", std::make_shared<Scene_Menu>(m_game));
}

// void Scene_Zelda::setPaused(bool pause) {
//     m_paused = pause;
// }

void Scene_Zelda::sRender() {
  m_game->window().clear(sf::Color(255, 192, 122));
  sf::RectangleShape tick({1.0f, 6.0f});
  tick.setFillColor(sf::Color::Black);

  // draw all Entity textures / animations
  if (m_drawTextures) {
    for (const auto &entity : m_entityManager.getEntities()) {
      auto &transform = entity->get<CTransform>();
      sf::Color c = sf::Color::White;
      if (entity->has<CInvincibility>()) {
        c = sf::Color(255, 255, 255, 128);
      }
      if (entity->has<CAnimation>()) {
        auto &animation = entity->get<CAnimation>().animation;
        animation.getSprite().setRotation(transform.angle);
        animation.getSprite().setPosition(transform.pos.x, transform.pos.y);
        animation.getSprite().setScale(transform.scale.x, transform.scale.y);
        animation.getSprite().setColor(c);
        m_game->window().draw(animation.getSprite());
      }
    }

    for (const auto &entity : m_entityManager.getEntities()) {
      auto &transform = entity->get<CTransform>();
      if (entity->has<CHealth>()) {
        auto &h = entity->get<CHealth>();
        vec2 size(64, 6);
        sf::RectangleShape rect({size.x, size.y});
        rect.setPosition(transform.pos.x - 32, transform.pos.y - 48);
        rect.setFillColor(sf::Color(96, 96, 96));
        rect.setOutlineColor(sf::Color::Black);
        rect.setOutlineThickness(2);
        m_game->window().draw(rect);

        float ratio = (float)(h.current) / h.max;
        size.x *= ratio;
        rect.setSize({size.x, size.y});
        rect.setFillColor(sf::Color(255, 0, 0));
        rect.setOutlineThickness(0);
        m_game->window().draw(rect);

        for (int i = 0; i < h.max; i++) {
          tick.setPosition(rect.getPosition() +
                           sf::Vector2f(i * 64 * 1.0 / h.max, 0));
          m_game->window().draw(tick);
        }
      }
    }
  }

  // draw all Entity collision bounding boxes with a rectangle shape
  if (m_drawCollision) {
    // draw bounding box
    sf::CircleShape dot(4);
    for (const auto &entity : m_entityManager.getEntities()) {
      if (entity->has<CBoundingBox>()) {
        auto &box = entity->get<CBoundingBox>();
        auto &entityPosition = entity->get<CTransform>().pos;
        sf::RectangleShape rect;
        rect.setSize(sf::Vector2f(box.size.x - 1, box.size.y - 1));
        rect.setOrigin(sf::Vector2f(box.halfSize.x, box.halfSize.y));
        rect.setPosition(entityPosition.x, entityPosition.y);
        rect.setFillColor(sf::Color(0, 0, 0, 0));
        if (box.blockMove && box.blockVision) {
          rect.setOutlineColor(sf::Color::Black);
        }
        if (box.blockMove && !box.blockVision) {
          rect.setOutlineColor(sf::Color::Blue);
        }
        if (!box.blockMove && box.blockVision) {
          rect.setOutlineColor(sf::Color::Red);
        }
        if (!box.blockMove && !box.blockVision) {
          rect.setOutlineColor(sf::Color::White);
        }
        rect.setOutlineThickness(1);
        m_game->window().draw(rect);

        // draw line between player and npc
        if (!player())
          continue;
        if (entity->tag() == "npc") {
          auto &ePos = entity->get<CTransform>().pos;
          auto view = m_game->window().getView().getCenter();
          if (ePos.x >= view.x - (float)width() / 2.0 &&
              ePos.x <= view.x + (float)width() / 2.0 &&
              ePos.y >= view.y - (float)height() / 2.0 &&
              ePos.y <= view.y + (float)height() / 2.0) {
            drawLine(player()->get<CTransform>().pos,
                     entity->get<CTransform>().pos);
          }
        }
      }

      // draw patrol points
      if (entity->has<CFollowPlayer>()) {
        auto &h = entity->get<CFollowPlayer>().home;
        dot.setPosition(h.x, h.y);
        m_game->window().draw(dot);
      }
      if (entity->has<CPatrol>()) {
        for (auto p : entity->get<CPatrol>().positions) {
          vec2 r = getRoomXY(entity->get<CTransform>().pos);
          vec2 pos = getPosition(r.x, r.y, p.x, p.y);
          dot.setPosition(pos.x, pos.y);
          m_game->window().draw(dot);
        }
      }
    }
  }

  // draw the grid so that can easily debug
  if (m_drawGrid) {
    float leftX = m_game->window().getView().getCenter().x - width() / 2.0;
    float rightX = leftX + width() + m_gridSize.x;
    float nextGridX = leftX - ((int)leftX % (int)m_gridSize.x);
    float topY = m_game->window().getView().getCenter().y - height() / 2.0;
    float bottomY = topY + height() + m_gridSize.y;
    float nextGridY = topY - ((int)topY % (int)m_gridSize.x);

    // draw room coordinate
    auto p = player();
    if (p) {
      vec2 r = getRoomXY(p->get<CTransform>().pos);
      m_gridText.setString("room \n" + std::to_string((int)r.x) + " " +
                           std::to_string((int)r.y));
      m_gridText.setPosition(leftX + m_gridSize.x + 3, topY + m_gridSize.y / 2);
      m_game->window().draw(m_gridText);
    }

    for (float x = nextGridX; x < rightX; x += m_gridSize.x) {
      drawLine(vec2(x, topY), vec2(x, bottomY));
    }

    for (float y = nextGridY; y < bottomY; y += m_gridSize.y) {
      drawLine(vec2(leftX, y), vec2(rightX, y));

      for (float x = nextGridX; x < rightX; x += m_gridSize.x) {
        int w = width();
        int h = height();
        std::string xCell =
            std::to_string(((((int)x % w) + w) % w) / (int)m_gridSize.x);
        std::string yCell =
            std::to_string(((((int)y % h) + h) % h) / (int)m_gridSize.y);
        m_gridText.setString("(" + xCell + "," + yCell + ")");
        m_gridText.setPosition(x + 3, y + 2);
        m_game->window().draw(m_gridText);
      }
    }
  }
}

std::shared_ptr<Entity> Scene_Zelda::player() {
  for (auto e : m_entityManager.getEntities("player")) {
    return e;
  }
  return nullptr;
}

void Scene_Zelda::changePlayerStateTo(const std::string &state,
                                      const vec2 &facing) {}

vec2 Scene_Zelda::windowToWorld(const vec2 &pos) {
  auto view = m_game->window().getView();
  float wx = view.getCenter().x - width() / 2.0f;
  float wy = view.getCenter().y - height() / 2.0f;
  return {pos.x + wx, pos.y + wy};
}

void Scene_Zelda::sDrag() {
  for (const auto &entity : m_entityManager.getEntities()) {
    if (entity->has<CDraggable>() && entity->get<CDraggable>().dragging) {
      vec2 wPos = windowToWorld(m_mousePos);
      entity->get<CTransform>().pos = wPos;
      entity->get<CBoundingBox>().center = wPos;
    }
    //        if (entity->has<CDraggable>()) {
    //            if (entity->get<CDraggable>().dragging) {
    //                vec2 wPos = windowToWorld(m_mousePos);
    //                entity->get<CTransform>().pos = wPos;
    //                entity->get<CBoundingBox>().center = wPos;
    //            }
    //        }
  }
}

vec2 Scene_Zelda::getRoomXY(const vec2 &pos) {
  auto winSize = m_game->window().getSize();
  int roomX = static_cast<int>(pos.x) / static_cast<int>(winSize.x);
  int roomY = static_cast<int>(pos.y) / static_cast<int>(winSize.y);
  if (pos.x < 0)
    roomX--;
  if (pos.y < 0)
    roomY--;
  return {(float)roomX, (float)roomY};
}

vec2 Scene_Zelda::gridToMidPixel(float gridX, float gridY,
                                std::shared_ptr<Entity> entity) {
  // This function takes in a grid (x,y) position and an Entity
  //       Return a Vec2 indicating where the CENTER position of the Entity
  //       should be You must use the Entity's Animation size to position it
  //       correctly The size of the grid width and height is stored in
  //       m_gridSize.x and m_gridSize.y The bottom-left corner of the Animation
  //       should aligh with the bottom left of the grid cell
  sf::Vector2 windowSize = m_game->window().getSize();
  float positionByGridX = m_gridSize.x * gridX;
  float positionByGridY = m_gridSize.y * gridY;
  vec2 spriteSize = entity->get<CAnimation>().animation.getSize();
  vec2 result = vec2((positionByGridX + spriteSize.x / 2),
                     (positionByGridY + spriteSize.y / 2));
  return result;
}
