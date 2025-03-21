import numpy as np
from itertools import combinations

def enumerate_vertices(A, b):
    """
    Enumerate vertices of a convex polytope defined by Ax <= b
    
    Parameters:
    A : numpy.ndarray
        Matrix of hyperplane coefficients (m constraints × n dimensions)
    b : numpy.ndarray
        Vector of hyperplane constants (m constraints)
        
    Returns:
    numpy.ndarray
        Array of vertices (k vertices × n dimensions)
    """
    m, n = A.shape  # m constraints, n dimensions
    vertices = []
    
    # For each possible combination of n hyperplanes
    for indices in combinations(range(m), n):
        # Extract the selected hyperplanes
        A_sub = A[indices]
        b_sub = b[indices]
        
        try:
            # Solve the system of equations A_sub * x = b_sub
            x = np.linalg.solve(A_sub, b_sub)
            
            # Check if the point satisfies all constraints
            if np.all(A @ x <= b + 1e-10):  # Small tolerance for numerical stability
                vertices.append(x)
        except np.linalg.LinAlgError:
            # Skip if the equations are singular
            continue
    
    # Convert to numpy array and remove duplicates
    if vertices:
        vertices = np.array(vertices)
        vertices = np.unique(vertices, axis=0)
        return vertices
    else:
        return np.array([])

def get_bounding_box(vertices):
    """
    Compute the axis-aligned bounding box of the polytope vertices
    
    Parameters:
    vertices : numpy.ndarray
        Array of vertices (k vertices × n dimensions)
        
    Returns:
    tuple
        (min_coords, max_coords) defining the bounding box corners
    """
    if len(vertices) == 0:
        return None, None
        
    min_coords = np.min(vertices, axis=0)
    max_coords = np.max(vertices, axis=0)
    
    return min_coords, max_coords

def enumerate_integer_points(min_coords, max_coords):
    """
    Enumerate all integer points within the bounding box
    
    Parameters:
    min_coords : numpy.ndarray
        Minimum coordinates of the bounding box
    max_coords : numpy.ndarray
        Maximum coordinates of the bounding box
        
    Returns:
    numpy.ndarray
        Array of integer points within the bounding box
    """
    if min_coords is None or max_coords is None:
        return np.array([])
        
    # Round the coordinates to ensure we capture all relevant integer points
    min_coords = np.floor(min_coords).astype(int)
    max_coords = np.ceil(max_coords).astype(int)
    
    # Generate ranges for each dimension
    ranges = [range(min_coord, max_coord + 1) 
             for min_coord, max_coord in zip(min_coords, max_coords)]
    
    # Generate all combinations of coordinates
    points = np.array(list(np.ndindex(*[len(r) for r in ranges])))
    
    # Transform indices to actual coordinates
    for i, r in enumerate(ranges):
        points[:, i] = points[:, i] + r.start
        
    return points
