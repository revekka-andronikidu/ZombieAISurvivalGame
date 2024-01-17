# Zombie Survival Game

## Description
Welcome to the Zombie Survival AI Agent project! In this repository, you'll find the code for an AI agent designed to navigate and survive in a challenging zombie-infested world. This project was a part of my Gameplay Programming course.

The agent has limited information about its surroundings. It has a rough knowledge about the world in general and the entities within its line of sight. The goal of the agent is to survive for as long as possible, aiming for a high score. To achieve this, the agent needs to actively gather and use helpful items scattered throughout the world and use them to handle the threat of roaming enemy zombies.

https://github.com/Howest-DAE-GD/gpp-zombiesurvivalgame-revekka-andronikidu/assets/133512944/85d658e4-c1cb-49b1-8d7a-161097b80a2d

_Sped up (2x) version of gameplay_

## Implementation
### Decision making
I chose the behavior tree for my decision making in the AI Zombie survival game because of its versatility and ease of implementation. Behavior trees provide a hierarchical structure that allows me to break down complex decision-making processes into manageable and modular components. In the context of a zombie survival game, where the agent needs to navigate a dynamic and unpredictable environment, behavior trees offer a clear and intuitive way to represent the logic behind the AI's actions.

![image](https://github.com/Howest-DAE-GD/gpp-zombiesurvivalgame-revekka-andronikidu/assets/133512944/f554c791-3f18-484a-9b30-14ac55025f0d)

The adaptability of behavior trees makes them well-suited for scenarios where the AI must respond to various changing conditions. In the zombie survival game, the agent's decision-making process involves a range of tasks, such as searching for resources,and avoiding and destroying zombies. By organizing these tasks into a behavior tree, I can easily adjust and extend the AI's behavior without rewriting the entire decision-making system.

Furthermore, behavior trees excel in handling priorities and decision sequences, allowing the AI to make informed choices based on the current game state. This hierarchical structure enables me to create a logical flow of actions for the AI, ensuring that it responds appropriately to different situations in the game. Ultimately, the decision to implement a behavior tree aligns with the need for a scalable, adaptable, and efficient AI system in the context of a challenging and dynamic zombie survival environment.

### Exploration

To create efficient world exploration, I divided the environment into small cells. The exploration process starts from the central point of the world, where the agent is assigned a set of cells to investigate within a defined radius. Upon completing the exploration of all cells within this initial radius, the exploration range expands incrementally. This iterative approach allows the agent to systematically continue its exploration, extending the radius until the entire world has been thoroughly discovered. This strategic method optimizes the agent's navigation, ensuring a comprehensive exploration of the environment in a systematic and organized manner.

https://github.com/Howest-DAE-GD/gpp-zombiesurvivalgame-revekka-andronikidu/assets/133512944/93deec9c-eac7-4c13-87f3-248074e925a7

_Showcase of world exploration in debug mode with no enemies_


## Conclusion
The Zombie Survival Game project demonstrates the successful implementation of AI Agent, the agent's ability to navigate, survive, and strategically handle encounters with zombies with a behavior tree as the backbone of the decision-making process. The strategic choices made in both decision-making and world exploration methods significantly contribute to the overall effectiveness of the AI agent.

The hierarchical structure of the behavior tree not only breaks down the complex decision-making into manageable components but also provides a clear and intuitive representation of the AI's logic in navigating the dynamic and unpredictable zombie-infested environment. The system's proficiency in handling priorities and decision sequences ensures that the AI makes informed choices based on real-time game states. Ultimately, the implementation of behavior trees aligns perfectly with the demands of scalability, adaptability, and efficiency required in the challenging context of a zombie survival game.

Simultaneously, the approach taken in world exploration demonstrates a systematic and organized strategy. By dividing the environment into small cells, the exploration process efficiently emanates from the central point of the world. The iterative expansion of the exploration radius allows the agent to methodically uncover the entire world. This systematic method optimizes the agent's navigation, ensuring comprehensive coverage of the environment in a strategic and organized manner.



 
