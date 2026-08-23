import numpy as np
from stable_baselines3 import PPO

def export_matrix(file, matrix):
    rows, cols = matrix.shape
    file.write(f"{rows} {cols}\n")
    data_str = " ".join([str(val) for val in matrix.flatten()])
    file.write(f"{data_str}\n")

def export_model_instance(model, out_txt_path):
    policy_net = model.policy.mlp_extractor.policy_net
    action_net = model.policy.action_net

    import torch.nn as nn
    
    layers = []
    for module in policy_net:
        if isinstance(module, nn.Linear):
            layers.append(module)
    layers.append(action_net)

    with open(out_txt_path, "w") as f:
        # Write number of layers
        f.write(f"{len(layers)}\n")
        
        for layer in layers:
            weight = layer.weight.detach().cpu().numpy()
            bias = layer.bias.detach().cpu().numpy().reshape(-1, 1)
            
            export_matrix(f, weight)
            export_matrix(f, bias)

def export_model(zip_path, out_txt_path):
    print(f"Loading {zip_path}...")
    model = PPO.load(zip_path)
    export_model_instance(model, out_txt_path)
    print(f"Successfully exported weights to {out_txt_path}")

if __name__ == "__main__":
    export_model("ppo_soccer_vectorized.zip", "ppo_brain.txt")
