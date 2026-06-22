from stable_baselines3 import PPO
from stable_baselines3.common.callbacks import CheckpointCallback
from stable_baselines3.common.vec_env import VecMonitor
from env_wrapper import SoccerGymWrapper

def main():
    print("Initializing Vectorized Environment...")
    env = SoccerGymWrapper()
    # Wrap the environment so Stable Baselines3 logs episode rewards and lengths
    env = VecMonitor(env)
    print(f"Environment initialized with {env.num_envs} parallel arenas.")

    # Use n_steps=1024. With 600 self-play environments, this means 614,400 steps per update.
    # Batch size of 16384 provides stable, fast gradients for this massive scale.
    policy_kwargs = dict(net_arch=dict(pi=[128, 128], vf=[128, 128]))
    model = PPO("MlpPolicy", env, verbose=1, n_steps=1024, batch_size=16384, ent_coef=0.005, policy_kwargs=policy_kwargs)
    
    # Automatically save a backup every 500,000 timesteps
    checkpoint_callback = CheckpointCallback(
        save_freq=max(500_000 // env.num_envs, 1),
        save_path="./models/",
        name_prefix="ppo_soccer"
    )

    print("Starting massively parallel training...")
    print("Press Ctrl+C at any time to safely stop training and save the model!")
    
    try:
        model.learn(total_timesteps=100_000_000, callback=checkpoint_callback)
    except KeyboardInterrupt:
        print("\nTraining interrupted by user. Saving current brain...")
    
    model.save("ppo_soccer_vectorized")
    print("Model successfully saved to ppo_soccer_vectorized.zip")

if __name__ == "__main__":
    main()
