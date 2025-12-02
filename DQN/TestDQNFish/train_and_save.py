#!/usr/bin/env python3


import argparse
import math
import random
from collections import deque, namedtuple
import numpy as np
import torch
import torch.nn as nn
import torch.optim as optim
import time
import os

# --------------------------- Environnement ---------------------------
class FishEnv:
    def __init__(self, N=50, box_size=10.0, v=0.03, R=1.0, theta_max=math.pi/6, m_actions=7, seed=None):
        self.N = N
        self.box_size = box_size
        self.v = v
        self.R = R
        self.theta_max = theta_max
        self.m = m_actions
        if seed is not None:
            np.random.seed(seed)
            random.seed(seed)
            torch.manual_seed(seed)
        self.actions = np.linspace(-self.theta_max, self.theta_max, self.m)
        self.reset()
    
    def reset(self):
        self.pos = np.random.rand(self.N, 2) * self.box_size
        angles = np.random.rand(self.N) * 2 * math.pi
        self.head = np.stack([np.cos(angles), np.sin(angles)], axis=1)
        self.neighbor_counts = self._compute_neighbor_counts()
        states = self._compute_states()
        return states
    
    def step(self, actions_idx):
        n_before = self.neighbor_counts.copy()
        deltas = self.actions[np.array(actions_idx)]
        angles = np.arctan2(self.head[:,1], self.head[:,0]) + deltas
        self.head = np.stack([np.cos(angles), np.sin(angles)], axis=1)
        self.pos += self.v * self.head
        self.pos = np.mod(self.pos, self.box_size)
        self.neighbor_counts = self._compute_neighbor_counts()
        states = self._compute_states()
        rewards = (self.neighbor_counts >= n_before).astype(np.float32)
        return states, rewards, self.neighbor_counts, {}
    
    def _compute_neighbor_counts(self):
        dpos = self.pos[:, None, :] - self.pos[None, :, :]
        dpos = (dpos + self.box_size/2) % self.box_size - self.box_size/2
        d2 = np.sum(dpos**2, axis=-1)
        within = (d2 <= self.R**2) & (d2 > 0.0)
        counts = within.sum(axis=1)
        return counts
    
    def _compute_states(self):
        dpos = self.pos[:, None, :] - self.pos[None, :, :]
        dpos = (dpos + self.box_size/2) % self.box_size - self.box_size/2
        d2 = np.sum(dpos**2, axis=-1)
        within = (d2 <= self.R**2) & (d2 > 0.0)
        states = np.zeros(self.N, dtype=np.float32)
        for i in range(self.N):
            idx = np.where(within[i])[0]
            if idx.size == 0:
                states[i] = 0.0
            else:
                mean_head = np.mean(self.head[idx], axis=0)
                a = math.atan2(mean_head[1], mean_head[0]) - math.atan2(self.head[i,1], self.head[i,0])
                a = (a + math.pi) % (2*math.pi) - math.pi
                states[i] = np.float32(a)
        return states

    def compute_order_parameter(self):
        mean_v = np.linalg.norm(np.sum(self.head, axis=0)) / self.N
        return mean_v

# --------------------------- Replay Buffer ---------------------------
Transition = namedtuple('Transition', ('s', 'a', 'r', 's2'))
class ReplayBuffer:
    def __init__(self, capacity=2000):
        self.capacity = capacity
        self.buffer = deque(maxlen=capacity)
    def push(self, *args):
        self.buffer.append(Transition(*args))
    def sample(self, batch_size):
        batch = random.sample(self.buffer, batch_size)
        return Transition(*zip(*batch))
    def __len__(self):
        return len(self.buffer)

# --------------------------- DQN model ---------------------------
class QNetwork(nn.Module):
    def __init__(self, input_dim=1, hidden=10, n_actions=7):
        super().__init__()
        self.net = nn.Sequential(
            nn.Linear(input_dim, hidden),
            nn.ReLU(),
            nn.Linear(hidden, n_actions)
        )
    def forward(self, x):
        return self.net(x)

# --------------------------- Agent ---------------------------
class DQNAgent:
    def __init__(self, m_actions=7, lr=0.01, gamma=0.99, device=None):
        self.m = m_actions
        self.device = device or ('cuda' if torch.cuda.is_available() else 'cpu')
        self.q = QNetwork(input_dim=1, hidden=10, n_actions=self.m).to(self.device)
        self.q_target = QNetwork(input_dim=1, hidden=10, n_actions=self.m).to(self.device)
        self.q_target.load_state_dict(self.q.state_dict())
        self.optimizer = optim.SGD(self.q.parameters(), lr=lr)
        self.gamma = gamma
        self.loss_fn = nn.MSELoss()
    
    def select_action(self, state, eps=0.1):
        if np.random.rand() < eps:
            return np.random.randint(0, self.m, size=state.shape[0])
        else:
            with torch.no_grad():
                s = torch.tensor(state.reshape(-1,1), dtype=torch.float32, device=self.device)
                qvals = self.q(s).cpu().numpy()
                return np.argmax(qvals, axis=1)
    
    def update(self, batch):
        device = self.device
        s = torch.tensor(np.array(batch.s).reshape(-1,1), dtype=torch.float32, device=device)
        a = torch.tensor(np.array(batch.a).reshape(-1,), dtype=torch.long, device=device)
        r = torch.tensor(np.array(batch.r).reshape(-1,1), dtype=torch.float32, device=device)
        s2 = torch.tensor(np.array(batch.s2).reshape(-1,1), dtype=torch.float32, device=device)
        qsa = self.q(s).gather(1, a.unsqueeze(1)).squeeze(1)
        with torch.no_grad():
            q_target_next = self.q_target(s2).max(dim=1)[0]
            target = (r.squeeze(1) + self.gamma * q_target_next).detach()
        loss = self.loss_fn(qsa, target)
        self.optimizer.zero_grad()
        loss.backward()
        self.optimizer.step()
        return loss.item()

# --------------------------- Training loop ---------------------------
def train_and_save(args):
    env = FishEnv(N=args.N, box_size=args.box_size, v=args.v, R=args.R, theta_max=args.theta_max, m_actions=args.m, seed=args.seed)
    agent = DQNAgent(m_actions=args.m, lr=args.lr, gamma=args.gamma)
    buffer = ReplayBuffer(capacity=args.replay_capacity)
    global_step = 0
    losses = []
    best_order = 0.0
    start_time = time.time()
    for ep in range(args.episodes):
        states = env.reset()
        ep_reward = 0.0
        eps = max(args.eps_end, args.eps_start - (ep / max(1, args.episodes))*(args.eps_start-args.eps_end))
        for t in range(args.steps):
            actions_idx = agent.select_action(states, eps=eps)
            next_states, rewards, neighbor_counts, _ = env.step(actions_idx)
            for i in range(env.N):
                buffer.push(states[i], int(actions_idx[i]), float(rewards[i]), next_states[i])
            states = next_states
            ep_reward += rewards.mean()
            global_step += 1
            if len(buffer) >= args.batch_size:
                batch = buffer.sample(args.batch_size)
                loss = agent.update(batch)
                losses.append(loss)
            if global_step % args.update_target_every == 0:
                agent.q_target.load_state_dict(agent.q.state_dict())
        order = env.compute_order_parameter()
        if order > best_order and args.save is not None:
            best_order = order
            torch.save(agent.q.state_dict(), args.save)
        if (ep+1) % args.log_interval == 0 or ep==0:
            elapsed = time.time() - start_time
            print(f"Ep {ep+1}/{args.episodes} | avg reward (per step, mean over agents): {ep_reward/args.steps:.4f} | order param: {order:.4f} | eps: {eps:.3f} | time: {elapsed:.1f}s")
    # final save
    if args.save is not None:
        torch.save(agent.q.state_dict(), args.save)
    print("Training finished. Model saved to:", args.save)

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--episodes', type=int, default=500, help='number of episodes')
    parser.add_argument('--steps', type=int, default=5000, help='steps per episode')
    parser.add_argument('--N', type=int, default=50, help='number of agents')
    parser.add_argument('--box_size', type=float, default=10.0)
    parser.add_argument('--v', type=float, default=0.03)
    parser.add_argument('--R', type=float, default=1.0)
    parser.add_argument('--theta_max', type=float, default=math.pi/6)
    parser.add_argument('--m', type=int, default=7, help='number of discrete actions')
    parser.add_argument('--replay_capacity', type=int, default=2000)
    parser.add_argument('--batch_size', type=int, default=32)
    parser.add_argument('--lr', type=float, default=0.01)
    parser.add_argument('--gamma', type=float, default=0.99)
    parser.add_argument('--update_target_every', type=int, default=100)
    parser.add_argument('--eps_start', type=float, default=1.0)
    parser.add_argument('--eps_end', type=float, default=0.01)
    parser.add_argument('--save', type=str, default='/mnt/data/fish_dqn.pth')
    parser.add_argument('--seed', type=int, default=42)
    parser.add_argument('--log_interval', type=int, default=10)
    return parser.parse_args()

if __name__ == '__main__':
    args = parse_args()
    os.makedirs(os.path.dirname(args.save), exist_ok=True)
    train_and_save(args)
