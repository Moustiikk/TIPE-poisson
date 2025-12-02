#!/usr/bin/env python3


import argparse
import math
import numpy as np
import torch
import pygame
import sys
import time

# Réutilisation légère des mêmes classes minimalistes (environnement léger + réseau)
class FishEnvVisual:
    def __init__(self, N=50, box_size=600, v=2.0, R=40.0, theta_max=math.pi/6, m_actions=7, seed=None):
        # note: here box_size in pixels for display; v in pixels per step
        self.N = N
        self.box_size = box_size
        self.v = v
        self.R = R
        self.theta_max = theta_max
        self.m = m_actions
        if seed is not None:
            np.random.seed(seed)
            torch.manual_seed(seed)
        self.actions = np.linspace(-self.theta_max, self.theta_max, self.m)
        self.reset()
    def reset(self):
        self.pos = np.random.rand(self.N, 2) * self.box_size
        angles = np.random.rand(self.N) * 2 * math.pi
        self.head = np.stack([np.cos(angles), np.sin(angles)], axis=1)
        self.neighbor_counts = self._compute_neighbor_counts()
        return self._compute_states()
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

class QNetworkSimple(torch.nn.Module):
    def __init__(self, input_dim=1, hidden=10, n_actions=7):
        super().__init__()
        self.net = torch.nn.Sequential(
            torch.nn.Linear(input_dim, hidden),
            torch.nn.ReLU(),
            torch.nn.Linear(hidden, n_actions)
        )
    def forward(self, x):
        return self.net(x)

def run_visual(args):
    device = 'cuda' if torch.cuda.is_available() else 'cpu'
    env = FishEnvVisual(N=args.N, box_size=args.box_size, v=args.v, R=args.R, theta_max=args.theta_max, m_actions=args.m, seed=args.seed)
    model = QNetworkSimple(input_dim=1, hidden=10, n_actions=args.m).to(device)
    model.load_state_dict(torch.load(args.model, map_location=device))
    model.eval()

    pygame.init()
    screen = pygame.display.set_mode((args.box_size, args.box_size))
    clock = pygame.time.Clock()
    states = env.reset()

    fish_color = (30, 144, 255)
    bg_color = (10, 10, 30)
    radius = 4

    running = True
    step = 0
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
        # choose actions by network
        with torch.no_grad():
            s = torch.tensor(states.reshape(-1,1), dtype=torch.float32, device=device)
            qvals = model(s).cpu().numpy()
            actions_idx = np.argmax(qvals, axis=1)
        states, rewards, nc, _ = env.step(actions_idx)
        screen.fill(bg_color)
        # draw fish as triangles indicating heading
        for i in range(env.N):
            x,y = env.pos[i]
            hx,hy = env.head[i]
            # triangle points
            p1 = (int(x + hx*8), int(y + hy*8))
            p2 = (int(x - hy*4), int(y + hx*4))
            p3 = (int(x + hy*4), int(y - hx*4))
            pygame.draw.polygon(screen, fish_color, (p1,p2,p3))
        pygame.display.flip()
        clock.tick(args.fps)
        step += 1
    pygame.quit()

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--model', type=str, default='/mnt/data/fish_dqn.pth', help='path to saved model')
    parser.add_argument('--N', type=int, default=50)
    parser.add_argument('--box_size', type=int, default=600)
    parser.add_argument('--v', type=float, default=2.0)
    parser.add_argument('--R', type=float, default=40.0)
    parser.add_argument('--theta_max', type=float, default=math.pi/6)
    parser.add_argument('--m', type=int, default=7)
    parser.add_argument('--seed', type=int, default=42)
    parser.add_argument('--fps', type=int, default=30)
    return parser.parse_args()

if __name__ == '__main__':
    args = parse_args()
    run_visual(args)
