import os
import glob
from stable_baselines3 import PPO
from stable_baselines3.common.callbacks import CheckpointCallback, BaseCallback
from stable_baselines3.common.vec_env import VecMonitor
from env_wrapper import SoccerGymWrapper
from export_model import export_model_instance

class OpponentPoolCallback(BaseCallback):
    def __init__(self, check_freq: int, pool_dir: str = os.path.join("models", "pool"), max_pool_size: int = 10, verbose: int = 1):
        super().__init__(verbose)
        self.check_freq = check_freq
        self.pool_dir = pool_dir
        self.max_pool_size = max_pool_size

    def _init_callback(self) -> None:
        os.makedirs(self.pool_dir, exist_ok=True)
        files = glob.glob(os.path.join(self.pool_dir, "snapshot_*.txt"))
        if not files:
            initial_path = os.path.join(self.pool_dir, "snapshot_0.txt")
            if self.verbose > 0:
                print(f"Opponent pool empty at start. Saving initial snapshot to {initial_path}")
            export_model_instance(self.model, initial_path)

    def _on_step(self) -> bool:
        if self.n_calls % self.check_freq == 0:
            snapshot_path = os.path.join(self.pool_dir, f"snapshot_{self.num_timesteps}.txt")
            if self.verbose > 0:
                print(f"Saving opponent pool snapshot to {snapshot_path}")
            export_model_instance(self.model, snapshot_path)
            self._cleanup_pool()
        return True

    def _cleanup_pool(self):
        files = glob.glob(os.path.join(self.pool_dir, "snapshot_*.txt"))
        if len(files) > self.max_pool_size:
            files.sort(key=lambda x: os.path.getmtime(x))
            while len(files) > self.max_pool_size:
                oldest = files.pop(0)
                if self.verbose > 0:
                    print(f"Removing oldest opponent snapshot (FIFO): {oldest}")
                try:
                    os.remove(oldest)
                except OSError:
                    pass

def main():
    print("Initializing Vectorized Environment...")
    env = SoccerGymWrapper()
    env = VecMonitor(env)
    print(f"Environment initialized with {env.num_envs} parallel arenas.")

    policy_kwargs = dict(net_arch=dict(pi=[128, 128], vf=[128, 128]))
    
    model = PPO("MlpPolicy", env, verbose=1, 
                n_steps=512,                
                batch_size=16384,
                learning_rate=5e-4,
                gamma=0.995,
                ent_coef=0.0,
                policy_kwargs=policy_kwargs)
    
    # Automatically save a backup every 500,000 timesteps
    save_freq = max(500_000 // env.num_envs, 1)
    checkpoint_callback = CheckpointCallback(
        save_freq=save_freq,
        save_path="./models/",
        name_prefix="ppo_soccer"
    )
    opponent_pool_callback = OpponentPoolCallback(
        check_freq=save_freq,
        pool_dir=os.path.join("models", "pool"),
        max_pool_size=10
    )

    print("Starting parallel training...")
    print("Press Ctrl+C at any time to safely stop training and save the model")
    
    try:
        model.learn(total_timesteps=30_000_000, callback=[checkpoint_callback, opponent_pool_callback])
    except KeyboardInterrupt:
        print("\nTraining interrupted by user. Saving current brain...")
    
    model.save("ppo_soccer_vectorized")
    print("Model successfully saved to ppo_soccer_vectorized.zip")

if __name__ == "__main__":
    main()
