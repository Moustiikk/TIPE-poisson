
Fichiers créés:
- /mnt/data/train_and_save.py   : script d'entrainement DQN et sauvegarde du modèle
- /mnt/data/simulate_pygame.py  : script de simulation visuelle (Pygame) chargeant le modèle sauvegardé

Exemples d'exécution (local) :
1) Entraînement (peut prendre du temps) :
    python3 /mnt/data/train_and_save.py --episodes 500 --steps 5000 --N 50 --save /mnt/data/fish_dqn.pth

2) Simulation visuelle (après avoir sauvegardé le modèle) :
    pip install pygame torch numpy
    python3 /mnt/data/simulate_pygame.py --model /mnt/data/fish_dqn.pth --N 50

Remarques :
- Ajuste les hyperparamètres dans la ligne de commande selon tes besoins.
- Le script d'entraînement sauvegarde le dernier modèle et le meilleur modèle observé (par order param).
