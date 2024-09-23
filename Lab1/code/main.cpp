#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <vector>
#include <list>
#include <random>

using namespace sf;
using namespace std;

class ECE_Centipede : public Sprite {
public:
    ECE_Centipede(Texture& headTexture, Texture& bodyTexture, int segmentsCount = 11)
        : headTexture(headTexture), bodyTexture(bodyTexture), direction(-1)
    {
        setTexture(headTexture);
        for (int i = 0; i < segmentsCount; ++i)
        {
            Texture& texture = (i == 0) ? headTexture : bodyTexture;
            Sprite segmentSprite(texture);

            const auto& size = segmentSprite.getLocalBounds();
            segmentSprite.setOrigin(size.width / 2.f, size.height / 2.f);

            segmentSprite.setPosition(800 + i * 20, 50);

            if (i == 0)
            {
                segmentSprite.setRotation(180);
            }

            segments.push_back({segmentSprite, 1});
        }
    }

    void update()
    {
        // Move all segments
        for (auto& segment : segments)
        {
            segment.first.move(2 * direction, 0);
        }

        // Check if the centipede head reaches the boundary
        auto& head = segments[0].first;
        if (direction == 1 && head.getPosition().x + head.getGlobalBounds().width >= 1036)
        {
            changeDirectionAndMoveDown();
        }
        else if (direction == -1 && head.getPosition().x <= 0)
        {
            changeDirectionAndMoveDown();
        }
    }

    void hit(int index)
    {
        segments[index].second--;
    }

    bool isSegmentDestroyed(int index) const
    {
        return segments[index].second <= 0;
    }

    void removeDestroyedSegments()
    {
        segments.erase(
            remove_if(segments.begin(), segments.end(),
                           [](const pair<Sprite, int>& segment)
                           {
                               return segment.second <= 0;
                           }),
            segments.end());
    }

    void draw(RenderWindow& window)
    {
        for (auto& segment : segments)
        {
            window.draw(segment.first);
        }
    }

    const vector<pair<Sprite, int>>& getSegments() const
    {
        return segments;
    }

private:
    vector<pair<Sprite, int>> segments;
    int direction;
    Texture& headTexture;
    Texture& bodyTexture;

    void changeDirectionAndMoveDown()
    {
        direction *= -1;

        if (direction == 1)
        {
            swap(segments.front(), segments.back());
            segments[0].first.setRotation(0);
        }
        else
        {
            swap(segments.back(), segments.front());
            segments[0].first.setRotation(180);
        }

        // Move all segments down
        for (auto& segment : segments)
        {
            segment.first.move(0, 20);
        }

        // Update head texture
        segments[0].first.setTexture(headTexture);
        for (int i = 1; i < segments.size(); ++i)
        {
            segments[i].first.setTexture(bodyTexture);
        }
    }
};

class ECE_LaserBlast : public Sprite {
public:
    ECE_LaserBlast(Texture& texture)
    {
        setTexture(texture);
    }

    void update()
    {
        move(0, -5);  // Move upwards
    }
};

class Mushroom {
public:
    Mushroom(const vector<Texture>& textures, float x, float y)
        : textures(textures), health(2)
    {
        sprite.setTexture(textures[0]);  // Start with the first texture
        sprite.setPosition(x, y);
    }

    Sprite& getSprite()
    {
        return sprite;
    }

    const Sprite& getSprite() const
    {
        return sprite;
    }

    bool isDestroyed() const
    {
        return health <= 0;
    }

    void hit()
    {
        health--;
        if (health == 1)
        {
            sprite.setTexture(textures[1]);  // Change to second texture on first hit
        }
        else if (health <= 0)
        {
            sprite.setColor(Color::Transparent);  // Make the mushroom invisible when destroyed
        }
    }

private:
    Sprite sprite;
    vector<Texture> textures;
    int health;
};

class Spider : public Sprite {
public:
    Spider(Texture& texture) : health(1)
    {
        setTexture(texture);
        setPosition(300, 300);
    }

    void update()
    {
        move(getRandomInt(-20, 20), getRandomInt(-10, 10));
    }

    void hit()
    {
        health--;
    }

    bool isDestroyed() const
    {
        return health <= 0;
    }

    void setRandomPosition()
    {
        float x = getRandomInt(0, 1000 - getGlobalBounds().width);
        float y = getRandomInt(0, 500 - getGlobalBounds().height);
        setPosition(x, y);
    }

private:
    int getRandomInt(int min, int max)
    {
        static random_device rd;
        static mt19937 gen(rd());
        uniform_int_distribution<> distr(min, max);
        return distr(gen);
    }

    int health;
};

int getRandomInt(int min, int max)
{
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distr(min, max);
    return distr(gen);
}

int main()
{
    const int SCREEN_WIDTH = 1036;
    const int SCREEN_HEIGHT = 569;
    RenderWindow window(VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Centipede Game");
    window.setFramerateLimit(60);

    // Load textures
    Texture centipedeHeadTexture;
    Texture centipedeBodyTexture;
    Texture mushroomTextures[2];
    Texture spaceshipTexture;
    Texture laserBlastTexture;
    Texture spiderTexture;
    Texture startupTexture;

    centipedeHeadTexture.loadFromFile("graphics/CentipedeHead.png");
    centipedeBodyTexture.loadFromFile("graphics/CentipedeBody.png");
    mushroomTextures[0].loadFromFile("graphics/Mushroom0.png");
    mushroomTextures[1].loadFromFile("graphics/Mushroom1.png");
    spaceshipTexture.loadFromFile("graphics/StarShip.png");
    laserBlastTexture.loadFromFile("graphics/Laser.png");
    spiderTexture.loadFromFile("graphics/Spider.png");
    startupTexture.loadFromFile("graphics/Startup Screen BackGround.png");

    // Create a vector of textures for the mushrooms
    vector<Texture> mushroomTextureVector(mushroomTextures, mushroomTextures + 2);

    // Create centipede with head and body textures
    ECE_Centipede centipede(centipedeHeadTexture, centipedeBodyTexture);

    // Create mushrooms
    list<Mushroom> mushrooms;
    for (int i = 0; i < 30; ++i)
    {
        float x = getRandomInt(0, SCREEN_WIDTH - 40);
        float y = getRandomInt(0, SCREEN_HEIGHT - 100);
        mushrooms.emplace_back(mushroomTextureVector, x, y);
    }

    // Create spaceship
    Sprite spaceship(spaceshipTexture);
    spaceship.setPosition(SCREEN_WIDTH / 2 - spaceship.getGlobalBounds().width / 2, SCREEN_HEIGHT - spaceship.getGlobalBounds().height);

    // Laser blasts
    list<ECE_LaserBlast> laserBlasts;

    // Create spider
    Spider spider(spiderTexture);

    // Score and lives
    int score = 0;
    int lives = 3;
    Font font;
    font.loadFromFile("fonts/KOMIKAP_.ttf");
    Text scoreText("Score: 0", font, 20);
    Text livesText("Lives: 3", font, 20);
    scoreText.setPosition(10, 10);
    livesText.setPosition(SCREEN_WIDTH - livesText.getGlobalBounds().width - 10, 10);

    bool gameStarted = false;

    // Fire cooldown variables
    float fireCooldown = 0.3f;
    float lastFireTime = 0.0f;

    Clock clock;
    bool gameOver = false;

    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
            {
                window.close();
            }

            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Enter)
            {
                gameStarted = true;
            }
        }

        // If the game is over, display the "Game Over" screen
        if (gameOver)
        {
            window.clear();
            Text gameOverText("Game Over", font, 40);
            gameOverText.setPosition(SCREEN_WIDTH / 2 - gameOverText.getGlobalBounds().width / 2, SCREEN_HEIGHT / 2);
            window.draw(gameOverText);
            window.display();
            continue;  // Skip the rest of the game logic
        }

        // Normal game logic continues here
        if (!gameStarted)
        {
            window.clear();
            Sprite startupScreen(startupTexture);
            window.draw(startupScreen);
            window.display();
            continue;
        }

        float currentTime = clock.getElapsedTime().asSeconds();

        // Move spaceship
        if (Keyboard::isKeyPressed(Keyboard::Left))
        {
            spaceship.move(-5, 0);
        }

        if (Keyboard::isKeyPressed(Keyboard::Right))
        {
            spaceship.move(5, 0);
        }

        if (Keyboard::isKeyPressed(Keyboard::Up))
        {
            spaceship.move(0, -5);
        }

        if (Keyboard::isKeyPressed(Keyboard::Down))
        {
            spaceship.move(0, 5);
        }

        // Keep spaceship within the screen
        if (spaceship.getPosition().x < 0)
        {
            spaceship.setPosition(0, spaceship.getPosition().y);
        }

        if (spaceship.getPosition().x + spaceship.getGlobalBounds().width > SCREEN_WIDTH)
        {
            spaceship.setPosition(SCREEN_WIDTH - spaceship.getGlobalBounds().width, spaceship.getPosition().y);
        }

        if (spaceship.getPosition().y < 0)
        {
            spaceship.setPosition(spaceship.getPosition().x, 0);
        }

        if (spaceship.getPosition().y + spaceship.getGlobalBounds().height > SCREEN_HEIGHT)
        {
            spaceship.setPosition(spaceship.getPosition().x, SCREEN_HEIGHT - spaceship.getGlobalBounds().height);
        }

        // Keep spider within the screen
        if (spider.getPosition().x < 0)
        {
            spider.setPosition(0, spider.getPosition().y);
        }

        if (spider.getPosition().x + spider.getGlobalBounds().width > SCREEN_WIDTH)
        {
            spider.setPosition(SCREEN_WIDTH - spider.getGlobalBounds().width, spider.getPosition().y);
        }

        if (spider.getPosition().y < 0)
        {
            spider.setPosition(spider.getPosition().x, 0);
        }

        if (spider.getPosition().y + spider.getGlobalBounds().height > SCREEN_HEIGHT)
        {
            spider.setPosition(spider.getPosition().x, SCREEN_HEIGHT - spider.getGlobalBounds().height);
        }

        // Laser firing logic with cooldown
        if (Keyboard::isKeyPressed(Keyboard::Space) && (currentTime - lastFireTime > fireCooldown))
        {
            laserBlasts.push_back(ECE_LaserBlast(laserBlastTexture));
            laserBlasts.back().setPosition(spaceship.getPosition().x - 20, spaceship.getPosition().y);
            lastFireTime = currentTime;
        }

        // Update laser blasts
        for (auto it = laserBlasts.begin(); it != laserBlasts.end(); )
        {
            it->update();
            bool hitSomething = false;

            // Check for collisions with mushrooms
            for (auto& mushroom : mushrooms)
            {
                if (it->getGlobalBounds().intersects(mushroom.getSprite().getGlobalBounds()))
                {
                    mushroom.hit();
                    hitSomething = true;
                    score += 10;
                }
            }

            // Check for collisions with spider
            if (it->getGlobalBounds().intersects(spider.getGlobalBounds()))
            {
                spider.hit();
                if (spider.isDestroyed())
                {
                    score += 100;
                    spider.setRandomPosition();
                }

                hitSomething = true;
            }

            // Check for collisions with centipede segments
            const auto& centipedeSegments = centipede.getSegments();
            for (int i = 0; i < centipedeSegments.size(); ++i)
            {
                if (it->getGlobalBounds().intersects(centipedeSegments[i].first.getGlobalBounds()))
                {
                    centipede.hit(i);
                    if (centipede.isSegmentDestroyed(i))
                    {
                        score += 50;
                    }

                    hitSomething = true;
                }
            }

            // Remove the laser blast if it hit something or went off the screen
            if (hitSomething || it->getPosition().y < 0)
            {
                it = laserBlasts.erase(it);
            }
            else
            {
                ++it;
            }
        }

        centipede.removeDestroyedSegments();

        // Remove the spider if destroyed
        if (spider.isDestroyed())
        {
            // Optionally respawn the spider after some time
        }

        // Remove destroyed mushrooms
        mushrooms.remove_if([](const Mushroom& mushroom) { return mushroom.isDestroyed(); });

        centipede.update();
        spider.update();

        // Check for collisions with the spaceship
        if (spider.getGlobalBounds().intersects(spaceship.getGlobalBounds()))
        {
            spaceship.setPosition(SCREEN_WIDTH / 2 - spaceship.getGlobalBounds().width / 2, SCREEN_HEIGHT - spaceship.getGlobalBounds().height);  // Reset spaceship position
            lives--;
        }

        for (auto& mushroom : mushrooms)
        {
            if (spider.getGlobalBounds().intersects(mushroom.getSprite().getGlobalBounds()))
            {
                mushroom.hit();
            }
        }

        // Game over condition
        if (lives <= 0)
        {
            gameOver = true;
        }

        // Update score and lives text
        scoreText.setString("Score: " + to_string(score));
        livesText.setString("Lives: " + to_string(lives));

        window.clear();
        window.draw(spaceship);
        window.draw(spider);

        for (const auto& mushroom : mushrooms)
        {
            window.draw(mushroom.getSprite());
        }

        for (const auto& blast : laserBlasts)
        {
            window.draw(blast);
        }

        centipede.draw(window);

        // Draw score and lives
        window.draw(scoreText);
        window.draw(livesText);

        window.display();
    }

    return 0;
}