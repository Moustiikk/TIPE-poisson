import pandas as pd
import matplotlib.pyplot as plt

csv_file = input("Numéro du fichier CSV : ")
low_max = input("Seuil low (défaut 80) : ")
mid_max = input("Seuil mid (défaut 140) : ")

out_png = "scatters_plot/scatter_plot_" + csv_file + ".png"
csv_file = "results/result_calculation_" + csv_file + ".csv"

low_max = int(low_max) if low_max else 80
mid_max = int(mid_max) if mid_max else 140

df = pd.read_csv(csv_file)


low = df[df["population_size"] < low_max]
mid = df[(df["population_size"] >= low_max) & (df["population_size"] < mid_max)]
high = df[df["population_size"] >= mid_max]

plt.figure(figsize=(13, 7))
plt.grid(True, alpha=0.3)

# X = polarization, Y = rotation
plt.scatter(high["polarization"], high["rotation"],
            s=35, color="#C00000", label="high")   # rouge

plt.scatter(low["polarization"], low["rotation"],
            s=35, color="#70AD47", label="low")    # vert

plt.scatter(mid["polarization"], mid["rotation"],
            s=35, color="#ED7D31", label="mid")    # orange


plt.xlim(0, 1)
plt.ylim(0, 1)
plt.xlabel("Polarization")
plt.ylabel("Rotation")
plt.title("Comportement collectif en fonction de la taille de la population")
plt.legend(loc="center left", bbox_to_anchor=(1.02, 0.5), frameon=False)

plt.tight_layout()
plt.savefig(out_png, dpi=300, bbox_inches="tight")
plt.show()
