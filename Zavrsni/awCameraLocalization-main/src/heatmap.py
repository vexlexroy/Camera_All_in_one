import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from scipy.interpolate import griddata
from matplotlib.patches import Circle

# ----------------------------
# 1. Prepare the data
# ----------------------------
data = [
    (408.39, 239.44, 0.23),
    (260.62, 183.87, 10.34),
    (414.37, 149.40, 1.98),
    (341.24, 450.61, 6.64),
    (464.87, 429.92, 7.28),
    (264.77, 340.67, 0.73),
    (192.85, 210.46, 1.10),
    (380.89, 303.96, 1.91),
    (343.58, 189.11, 2.71),
    (254.85, 228.91, 5.09),
    (225.79, 233.72, 6.21),
    (303.67, 242.84, 1.57),
    (210.13, 229.99, 0.15),
    (213.79, 278.66, 2.87),
    (243.92, 247.45, 1.61),
    (246.85, 302.61, 0.65)
]







x, y, error = zip(*data)

# ----------------------------
# 2. Compute statistics
# ----------------------------
mean_error = np.mean(error)
std_error = np.std(error)
sigma_2_upper = mean_error + 2 * std_error
sigma_3_upper = mean_error + 3 * std_error
max_error = max(error)  # Used later for scaling

print(f"Mean Error: {mean_error:.2f}")
print(f"Standard Deviation: {std_error:.2f}")
print(f"2 Sigma Range: 95% chance error is below {sigma_2_upper:.2f} pixels")
print(f"3 Sigma Range: 99.7% chance error is below {sigma_3_upper:.2f} pixels")
print(f"Max Error: {max_error:.2f}")

# ----------------------------
# 3. Parameters for scaling
# ----------------------------
vmin = 0
vmax_heatmap = 30  # Fixed color scale (0=green, 30=red)
log_scale_factor = 1.0  # Logarithmic aggressiveness control

# ----------------------------
# 4. Create grid and interpolate
# ----------------------------
xi, yi = np.meshgrid(np.linspace(0, 680, 680), np.linspace(0, 480, 480))
er_interp = griddata((x, y), error, (xi, yi), method='cubic', fill_value=np.nan)
er_interp = np.nan_to_num(er_interp, nan=np.nanmin(er_interp))

# Apply logarithmic scaling to values above max_error
er_transformed = np.where(
    er_interp <= max_error,
    er_interp,  # Linear for values ≤ max_error
    max_error + np.log1p(log_scale_factor * (er_interp - max_error))  # Log scaling
)
er_transformed = np.clip(er_transformed, vmin, vmax_heatmap)

# ----------------------------
# 5. Plot the heatmap and points
# ----------------------------
plt.figure(figsize=(8, 6))
ax = sns.heatmap(
    er_transformed,
    cmap='RdYlGn_r',
    cbar=True,
    xticklabels=False,
    yticklabels=False,
    vmin=vmin,
    vmax=vmax_heatmap
)

# Add circles for data points (colored relative to 0-30 scale)
cmap = plt.get_cmap('RdYlGn_r')
for (xi_val, yi_val, err_val) in data:
    norm_val = err_val / vmax_heatmap  # Normalize to 0-30
    face_color = cmap(norm_val)
    circle = Circle(
        (xi_val, yi_val),
        radius=err_val,
        facecolor=face_color,
        edgecolor='black',
        linewidth=1.0
    )
    ax.add_patch(circle)

ax.set_title(
    f"Reprojection Error Heatmap\n"
    f"Linear ≤ {max_error:.1f}, Log > {max_error:.1f} (Scale 0-30)\n"
    f"Log Scale Factor = {log_scale_factor}"
)
ax.set_xlabel("X")
ax.set_ylabel("Y")

plt.show()