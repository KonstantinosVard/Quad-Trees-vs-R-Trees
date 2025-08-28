# Quad Trees vs R-Trees: Performance Comparison for Geospatial Data

### MSc Data Science & Engineering · Complex Data Management Course · Project 4/4:  Quad Trees vs R-Trees

## Project Overview

This project implements and comprehensively compares two spatial data structures - Quad Trees and R-Trees - for efficient handling of geospatial data. The implementation includes optimized algorithms for range queries and k-nearest neighbor (k-NN) searches, with extensive performance analysis on real-world geographic datasets.

## Project Structure

```
├── src/                          # Source code directory
│   ├── HeapEntry.h              # Priority queue structure for k-NN queries
│   ├── Point.h & Point.cpp      # 2D point representation with distance calculations
│   ├── Rectangle.h & Rectangle.cpp # Rectangle class for spatial boundaries
│   ├── QuadTree.h & QuadTree.cpp # Quad Tree implementation
│   ├── RTree.h & RTree.cpp      # R-Tree implementation with bulk loading
│   └── (requires libmorton)     # External dependency for Z-order curves
├── data_analysis.ipynb          # Analysis of USA datasets and query structures
├── time_analysis.ipynb          # k-NN distribution analysis and performance insights
├── time_knn.ipynb               # k-NN query time vs number of neighbors analysis
├── range_times.ipynb            # Range query time vs query area analysis
├── report_english.pdf           # Comprehensive project report in English
├── report_greek.pdf             # Comprehensive project report in Greek
├── presentation_english.pptx    # Project presentation slides in English
└── presentation_greek.pptx      # Project presentation slides in Greek
```

## Implementation Details

### Core Data Structures:
- **Point Class**: 2D coordinates with Euclidean distance calculations
- **Rectangle Class**: Spatial boundaries with containment and intersection checks
- **HeapEntry Template**: Priority queue element for efficient k-NN searches

### Quad Tree Features:
- Capacity-based node splitting into four quadrants
- Dynamic insertion with point redistribution
- Efficient range and k-NN query implementations
- Configurable capacity parameter for performance tuning

### R-Tree Features:
- Bulk loading with two strategies: Z-order curves and STR (Sort-Tile-Recursive)
- Configurable min/max entries per node
- MBR (Minimum Bounding Rectangle) calculations
- Hierarchical structure with leaves and internal nodes

### Performance Optimization:
- Hyperparameter tuning for both structures
- Space transformation experiments (PCA rotations)
- Statistical analysis of tree characteristics vs query performance
- Comparison against naive baseline methodologies

## Dataset Information

The project uses two real-world geographic datasets of US locations:

- **T2 Dataset**: 2,280,427 points, x ∈ [-124.7595, -66.9875], y ∈ [24.5219, 49.1668]
- **T5 Dataset**: 5,043,188 points, x ∈ [-124.73, -66.9887], y ∈ [24.5635, 49.1611]

Query files contain 10,000 range queries each, with varying sizes (0.01%, 0.05%, 0.1%, 0.5%, 1% of total area).

## Performance Results

### k-NN Query Performance (T2 dataset, k=3):
- **Naïve method**: 240.33 μsec/query
- **Quad Tree**: 44.59 μsec/query (5.39× faster)
- **R-Tree (Z-order)**: 151.38 μsec/query (1.59× faster)
- **R-Tree (STR)**: 99.84 μsec/query (2.41× faster)

### Key Findings:
- Quad Trees outperform R-Trees for k-NN queries across all tested configurations
- R-Trees with STR packing show better performance than Z-order for most cases
- Both tree structures show linear scaling with query size/neighbors
- Space transformation via rotation showed statistically significant but practically small improvements (~2.5%)

## Analysis Notebooks

1. **data_analysis.ipynb**: Exploratory analysis of dataset distributions and query structures
2. **time_analysis.ipynb**: Investigation of k-NN performance distribution and "hard" regions
3. **time_knn.ipynb**: Analysis of k-NN query time vs number of neighbors
4. **range_times.ipynb**: Analysis of range query time vs query area


## Key Insights

1. **Quad Trees** excel in k-NN queries due to their simpler and more stable topology
2. **R-Trees** perform better for range queries, especially as query area increases
3. **Tree statistics** (min_depth, max_depth, leaf distribution) correlate with query performance
4. **Space transformation** through rotation can yield marginal but statistically significant improvements
5. **STR packing** generally outperforms Z-order sorting for R-Tree construction

## Author

**Konstantinos Vardakas**  

---

*This project demonstrates the practical implementation and comparative analysis of spatial data structures for efficient geospatial query processing, highlighting the trade-offs between different indexing strategies for various query types.*
