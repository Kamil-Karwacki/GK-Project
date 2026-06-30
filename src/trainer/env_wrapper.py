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
    def __init__(self, pool_dir=os.path.join("models", "pool")):
        self.pool_dir = pool_dir
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

    def randomize_opponents(self, indices=None):
        if not os.path.exists(self.pool_dir):
            return
        files = [os.path.join(self.pool_dir, f) for f in os.listdir(self.pool_dir) if f.endswith(".txt")]
        if not files:
            return
        files.sort(key=lambda x: os.path.getmtime(x))
        n = len(files)
        # Weight towards newer snapshots with linear weighting
        weights = np.arange(1, n + 1, dtype=np.float64)
        weights /= weights.sum()

        if indices is None:
            indices = range(self.num_envs)

        for idx in indices:
            chosen = np.random.choice(files, p=weights)
            self.cpp_env.set_opponent(int(idx), chosen)
            if idx == 0 or len(indices) <= 5:
                print(f"[OpponentPool] Arena {idx} switched opponent to {os.path.basename(chosen)}")

    def reset(self):
        obs = self.cpp_env.reset()
        self.randomize_opponents()
        return obs

    def step_async(self, actions):
        self.actions = actions

    def step_wait(self):
        obs, rewards, dones, infos = self.cpp_env.step(self.actions)
        done_indices = [i for i, d in enumerate(dones) if d]
        if done_indices:
            self.randomize_opponents(indices=done_indices)
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
