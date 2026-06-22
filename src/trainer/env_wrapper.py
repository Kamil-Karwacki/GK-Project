import gymnasium as gym
import numpy as np
import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..', 'build'))

if os.name == 'nt':
    mingw_path = r"D:\mingw64\bin"
    if os.path.exists(mingw_path):
        os.add_dll_directory(mingw_path)

import soccer_engine
from stable_baselines3.common.vec_env import VecEnv

class SoccerGymWrapper(VecEnv):
    def __init__(self):
        self.cpp_env = soccer_engine.SoccerEnv()
        
        obs_size = self.cpp_env.observation_size()
        act_size = self.cpp_env.action_size()
        num_envs = self.cpp_env.num_envs()
        
        observation_space = gym.spaces.Box(
            low=-np.inf, high=np.inf, shape=(obs_size,), dtype=np.float32
        )
        action_space = gym.spaces.Box(
            low=-1.0, high=1.0, shape=(act_size,), dtype=np.float32
        )
        
        # Initialize VecEnv base class
        super().__init__(num_envs, observation_space, action_space)

    def reset(self):
        obs = self.cpp_env.reset()
        return obs

    def step_async(self, actions):
        self.actions = actions

    def step_wait(self):
        obs, rewards, dones, infos = self.cpp_env.step(self.actions)
        return obs, np.array(rewards), np.array(dones), infos

    def close(self):
        pass

    def get_attr(self, attr_name, indices=None):
        return [None] * self.num_envs

    def set_attr(self, attr_name, value, indices=None):
        pass

    def env_method(self, method_name, *method_args, indices=None, **method_kwargs):
        pass

    def env_is_wrapped(self, wrapper_class, indices=None):
        return [False] * self.num_envs
