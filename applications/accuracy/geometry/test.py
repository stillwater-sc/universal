# Define hyperplane constraints Ax <= b
A = np.array([[1, 0], [0, 1], [-1, 0], [0, -1]])  # Example: box constraints
b = np.array([1, 1, 1, 1])  # Example: unit square

# Get vertices
vertices = enumerate_vertices(A, b)

# Get bounding box
min_coords, max_coords = get_bounding_box(vertices)

# Get integer points
integer_points = enumerate_integer_points(min_coords, max_coords)
