# Dragon Locomotion Prototype

> A C++ Unreal Engine 5 gameplay prototype exploring expressive dragon locomotion inspired by the Spyro series.

The goal of this project is to design a highly responsive locomotion system featuring grounded movement, momentum-based flight, gliding, banking, and smooth animation transitions using modern Unreal Engine systems.

[![Engine](https://img.shields.io/badge/Unreal_Engine_5-0E1128?style=flat-square&logo=unreal-engine&logoColor=blue)]()
[![Language](https://img.shields.io/badge/C++-00599C?style=flat-square&logo=c%2B%2B&logoColor=white)]()
[![Status](https://img.shields.io/badge/Status-Incomplete%20-red)]()

## 📖 Overview

Soar: A Dragon Locomotion Prototype is a 3D game prototype that explores highly responsive locomotion system featuring grounded movement and momentum-based flight. This project was bought to life when mind kept thinking about the several possibilities of what the upcoming 2027 video game, Spyro: A Realm Beyond would be like. The concept of being able to fly at anytime inspired me to want to learn how to implement a satisfying flight system with my own creative liberties. The goal of the project prototype was to create a working demo that explored creature locomotion and movement programming pyhsics in Unreal Engine. While also having the potential to evolve the mechanic into a plugin for other developers to use.

## Contributions

### My Role: Gameplay Engineer

I was responsible for designing and implementing a custom movement system with the usage of C++ and Unreal's Enhanced Input system  for several of the core
systems that drive the game's moment-to-moment gameplay.

I developed 7 gameplay systems in Unreal Engine using C++:

- Locomotion State System
- Ground Movement
- Momentum-Based Flight
- Flight Charging
- Enhanced Input
- Camera Follow
- Movement Debugging

## Features

- Ground locomotion
- Momentum-based flight
- Gliding
- Landing transitions
- Animation Blueprint integration
- C++ movement component

---

## 🎮 Gameplay

In Soar: A Dragon Locomotion Prototype, the player uses WASD or Left Stick to move the player character in their desired direction. The player's goal is to experiement with the Dragon's mobility on the ground and air in the test environment. The player can charge [Right Shift/B], jump [Space/B], and enter flight [Enter/Right Trigger]. The goal of the prototype is to explore and learn about making an intuitive and seamless gameplay mechanic with vector-based momentum.

### Controls

#### Ground Controls

| Action | Keyboard | Xbox |
|---|---|---|
| Move | WASD | Left Stick |
| Camera | Mouse | Right Stick | 
| Charge | Right Shift | B | 
| Jump | Space | A | 
| TakeOff | Enter | Right Trigger | 

#### Flight Controls

| Action | Keyboard | Xbox |
|---|---|---|
| Pitch Up | W | Left Stick Down |
| Pitch Down | S | Left Stick Up |
| Roll Right | D | Left Stick Right |
| Roll Left | A | Left Stick Left |
| Camera | Mouse | Right Stick | 
| Dive | Right Shift | B | 

---

## 🛠️ Technical Implementation



---

## 🧩 Challenges & Solutions

### Challenge 1 — Ground/Flight Transition

**Problem:**  
One of the major challenges during development was accurately and smoothly transitioning the player character from ground to ariel and vice-verca. In the duration of a few tests, I discovered that the player character would get stuck in the flight state despite touching the ground to transition to the land state. This caused that iteration of the protoype to elicit a buggy and unpolished experience. 

**Solution:**  
To resolve this problem, I used Unreal Engine's logging to identify the MovementMode of the player. Through Unreal's logging, I diagnosed that Unreal's CharacterMoveComponent state was stuck in flight mode as opposed to transitioning. After, diagnosising the issue, I implemented a line trace from the player capsule to detect how far the player is from the ground. If the line trace hits the ground, then the player transitions into ground mode. 

**Result:**  
After solving this issue, I managed to keep adjusting the line tracer for better accuracy in detecting the ground. Resulting in seamless transitions, a polished gameplay experience and stronger foundation to build upon in future iterations. 

---

### Challenge 2 — Ground Velocity Perservation

**Problem:**  
During the development, I also encountered the challenge of preserving forward momentum from the 4 ground states (idle, walk, run, charge) and carrying it over into flight. As the player would only use a set speed, rather than initially using the speed before they transition into flight.


**Solution:**  
To solve this issue, I used Unreal's GetCharacterMovement() function to capture the speed before they transition into flight. In implementation I set GetCharacterMovement()-> MaxFlySpeed to the current FlightSpeed and clamped it to prevent an unreasonable amount of speed from being generated. 

**Result:**  
As a result, the player character is now able to carry speed from their last state. Allowing for the player to comfortably make change gameplay states with intention and satisfaction. While also strengthening the current prototype. 

## 🚀 What I Learned

- 
- 
- 

---

## 🔮 Future Improvements

- The next step I want to take with Dragon Locomotion prototype is to refine and and polish the flight mechanics. Essentially, I want to minimize the amount of bugs to enhance the overall gameplay experience. For example, when the player presses the takeoff button, they instantly lose forward momentum without flapping the dragon's wings. The goal with this fix is to ensure that the dragon can carry forward momentum, but loses speed (forward velocity drops to a minimum speed, not 0) when the player isn't engaging with the game's mechanics. Thus, resulting the dragon gliding downward to ground level. 
- Another improvement I'd like to make is explore and implement ways for the player to engage with the environment to create lift and generate more forward momentum. Employing the player to become an active participant while playing/testing the prototype.  
- Lastly, I'd like to focus on transforming my custom ground/ariel locomotion system into it own component system. That way I can create behavior that Unreal Engine does not provide. Ultimately, making my system reusable and compatible to become a plugin.  

---

## 🎥 Gameplay / Demo

**Itch.io Demo:** https://ishmael-kwayisi.itch.io/project-not-zelda

**Video Demo:** [Mini Gameplay Demo](Screenshots/Project-Not-Zelda-Arrow-Combat.mp4)

---

## 📸 Screenshots

### [Pause Game]

![PauseMenu](Screenshots/Project-Not-Zelda-Screenshot1.png)

### [Player combat]

![Player Combat](Screenshots/Project-Not-Zelda-Screenshot2.png)

### [Player Combat Arrow]

![Arrow](Screenshots/Project-Not-Zelda-Screenshot3.png)

### [Endgame]

![Endgame](Screenshots/Project-Not-Zelda-Screenshot4.png)

---
## Tech Stack
- Unreal Engine 5.8
- C++
- Enhanced Input
- Animation Blueprints
- Git LFS
---

## Planned Features
- [ ] Polish
- [ ]PLayer Engagement

## 📝 Documentation

This README was authored and is maintained by Ishmael Kwayisi to
document the project's development, technical implementation, and
solo contributions.

## License

The original source code developed for this project is licensed under the MIT License.

This license applies only to original work contained within this repository. Unreal Engine, third-party assets, animations, models, and other externally provided content are not covered by this license and remain subject to their respective licenses and terms of use.

Third-Party Content

This project uses Unreal Engine and third-party assets for development and demonstration purposes. Third-party content remains the property of its respective owners and is subject to the licenses under which it was provided.

The MIT License does not grant permission to use, reproduce, modify, or redistribute third-party content included with the project.