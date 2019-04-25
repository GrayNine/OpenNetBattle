/*! \brief Virus background uses Background class to animate and scroll
 */
 
#pragma once
#include <SFML/Graphics.hpp>
using sf::Texture;
using sf::Sprite;
using sf::IntRect;
using sf::Drawable;
#include <vector>
using std::vector;

#include "bnBackground.h"

class VirusBackground : public Background
{
public:
  VirusBackground();
  ~VirusBackground();

  virtual void Update(float _elapsed);

private:
  float x, y;
  float progress;
};

