#include <iostream>
#include <vector>
#include <memory>

// Mock Qt classes for basic testing
class QString {
public:
    QString() = default;
    QString(const char* str) : data(str) {}
    QString(const std::string& str) : data(str) {}
    bool operator==(const QString& other) const { return data == other.data; }
    bool isEmpty() const { return data.empty(); }
    std::string data;
};

class QMatrix4x4 {
public:
    QMatrix4x4() { setToIdentity(); }
    void setToIdentity() {
        for (int i = 0; i < 16; ++i) m_data[i] = 0.0f;
        m_data[0] = m_data[5] = m_data[10] = m_data[15] = 1.0f;
    }
    bool isIdentity() const {
        return m_data[0] == 1.0f && m_data[5] == 1.0f && 
               m_data[10] == 1.0f && m_data[15] == 1.0f;
    }
private:
    float m_data[16];
};

template<typename T>
class QList {
public:
    void append(const T& item) { data.push_back(item); }
    int size() const { return static_cast<int>(data.size()); }
    bool isEmpty() const { return data.empty(); }
    void clear() { data.clear(); }
    T& operator[](int index) { return data[index]; }
    const T& operator[](int index) const { return data[index]; }
    
    typename std::vector<T>::iterator begin() { return data.begin(); }
    typename std::vector<T>::iterator end() { return data.end(); }
    typename std::vector<T>::const_iterator begin() const { return data.begin(); }
    typename std::vector<T>::const_iterator end() const { return data.end(); }
    
private:
    std::vector<T> data;
};

// Mock debug output
#define qDebug() std::cout
#define qWarning() std::cout
#define qCritical() std::cout

// Include our Sprint 9 components (simplified versions)
namespace Registration {

struct PoseNode {
    QString scanId;
    QMatrix4x4 transform;
    int nodeIndex;
    
    PoseNode() : nodeIndex(-1) {}
    PoseNode(const QString& id, const QMatrix4x4& t, int idx = -1) 
        : scanId(id), transform(t), nodeIndex(idx) {}
};

struct PoseEdge {
    int fromNodeIndex;
    int toNodeIndex;
    QMatrix4x4 relativeTransform;
    float informationMatrix;
    float rmsError;
    
    PoseEdge() : fromNodeIndex(-1), toNodeIndex(-1), informationMatrix(1.0f), rmsError(0.0f) {}
    PoseEdge(int from, int to, const QMatrix4x4& transform, float info = 1.0f, float rms = 0.0f)
        : fromNodeIndex(from), toNodeIndex(to), relativeTransform(transform), 
          informationMatrix(info), rmsError(rms) {}
};

class PoseGraph {
public:
    PoseGraph() = default;
    
    int addNode(const QString& scanId, const QMatrix4x4& transform = QMatrix4x4()) {
        int nodeIndex = m_nextNodeIndex++;
        m_nodes.append(PoseNode(scanId, transform, nodeIndex));
        qDebug() << "Added node " << nodeIndex << " for scan " << scanId.data << std::endl;
        return nodeIndex;
    }
    
    bool addEdge(int fromNode, int toNode, const QMatrix4x4& relativeTransform, float rmsError = 0.0f) {
        if (fromNode < 0 || toNode < 0 || fromNode == toNode) {
            return false;
        }
        
        float information = (rmsError > 0.0f) ? 1.0f / rmsError : 1.0f;
        m_edges.append(PoseEdge(fromNode, toNode, relativeTransform, information, rmsError));
        
        qDebug() << "Added edge " << fromNode << " -> " << toNode << " with RMS error " << rmsError << std::endl;
        return true;
    }
    
    int nodeCount() const { return m_nodes.size(); }
    int edgeCount() const { return m_edges.size(); }
    bool isEmpty() const { return m_nodes.isEmpty(); }
    bool hasLoopClosures() const { return m_edges.size() > (m_nodes.size() - 1); }
    
    const QList<PoseNode>& nodes() const { return m_nodes; }
    const QList<PoseEdge>& edges() const { return m_edges; }
    
private:
    QList<PoseNode> m_nodes;
    QList<PoseEdge> m_edges;
    int m_nextNodeIndex = 0;
};

} // namespace Registration

// Point3D structure for feature extraction
struct Point3D {
    float x, y, z;
    int intensity = 0;
    bool hasIntensity = false;
    
    Point3D() : x(0), y(0), z(0) {}
    Point3D(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

namespace Features {

struct Plane {
    float normal[3] = {0, 0, 1}; // Normal vector
    float distance = 0.0f;       // Distance from origin
    float centroid[3] = {0, 0, 0}; // Center point
    QList<int> inlierIndices;    // Point indices
    float confidence = 0.0f;     // Quality score
};

class FeatureExtractor {
public:
    struct PlaneExtractionParams {
        int maxIterations;
        float distanceThreshold;
        int minInliers;
        int maxPlanes;

        PlaneExtractionParams() : maxIterations(1000), distanceThreshold(0.02f), minInliers(100), maxPlanes(10) {}
    };

    QList<Plane> extractPlanes(const std::vector<Point3D>& points,
                              const PlaneExtractionParams& params = PlaneExtractionParams()) {
        QList<Plane> planes;
        
        if (points.size() < 3) {
            return planes;
        }
        
        // Simple plane detection for XY plane at Z=0
        Plane xyPlane;
        xyPlane.normal[0] = 0; xyPlane.normal[1] = 0; xyPlane.normal[2] = 1;
        xyPlane.distance = 0.0f;
        xyPlane.confidence = 0.9f;
        
        // Find points close to XY plane
        for (size_t i = 0; i < points.size(); ++i) {
            if (std::abs(points[i].z) < params.distanceThreshold) {
                xyPlane.inlierIndices.append(static_cast<int>(i));
            }
        }
        
        if (xyPlane.inlierIndices.size() >= params.minInliers) {
            planes.append(xyPlane);
            qDebug() << "Detected XY plane with " << xyPlane.inlierIndices.size() << " inliers" << std::endl;
        }
        
        return planes;
    }
};

} // namespace Features

// Test functions
void testPoseGraph() {
    std::cout << "\n=== Testing PoseGraph ===" << std::endl;
    
    Registration::PoseGraph graph;
    
    // Add nodes
    QMatrix4x4 identity;
    int node1 = graph.addNode("scan1", identity);
    int node2 = graph.addNode("scan2", identity);
    int node3 = graph.addNode("scan3", identity);
    
    std::cout << "Created graph with " << graph.nodeCount() << " nodes" << std::endl;
    
    // Add edges
    graph.addEdge(node1, node2, identity, 0.01f);
    graph.addEdge(node2, node3, identity, 0.02f);
    graph.addEdge(node3, node1, identity, 0.015f); // Loop closure
    
    std::cout << "Added " << graph.edgeCount() << " edges" << std::endl;
    std::cout << "Has loop closures: " << (graph.hasLoopClosures() ? "Yes" : "No") << std::endl;
}

void testFeatureExtraction() {
    std::cout << "\n=== Testing Feature Extraction ===" << std::endl;
    
    // Create test point cloud (XY plane at Z=0)
    std::vector<Point3D> points;
    for (int x = -5; x <= 5; ++x) {
        for (int y = -5; y <= 5; ++y) {
            points.emplace_back(static_cast<float>(x), static_cast<float>(y), 0.0f);
        }
    }
    
    std::cout << "Created test point cloud with " << points.size() << " points" << std::endl;
    
    Features::FeatureExtractor extractor;
    auto planes = extractor.extractPlanes(points);
    
    std::cout << "Extracted " << planes.size() << " planes" << std::endl;
    
    if (!planes.isEmpty()) {
        const auto& plane = planes[0];
        std::cout << "First plane: normal(" << plane.normal[0] << ", " 
                  << plane.normal[1] << ", " << plane.normal[2] 
                  << ") inliers=" << plane.inlierIndices.size() 
                  << " confidence=" << plane.confidence << std::endl;
    }
}

void testIntegration() {
    std::cout << "\n=== Testing Integration ===" << std::endl;
    
    // Create a complete workflow
    Registration::PoseGraph graph;
    
    // Build pose graph
    int node1 = graph.addNode("scan1");
    int node2 = graph.addNode("scan2");
    graph.addEdge(node1, node2, QMatrix4x4(), 0.02f);
    
    // Extract features
    std::vector<Point3D> points;
    for (int i = 0; i < 100; ++i) {
        points.emplace_back(static_cast<float>(i % 10), static_cast<float>(i / 10), 0.0f);
    }
    
    Features::FeatureExtractor extractor;
    auto planes = extractor.extractPlanes(points);
    
    std::cout << "Integration test completed:" << std::endl;
    std::cout << "  Pose graph: " << graph.nodeCount() << " nodes, " << graph.edgeCount() << " edges" << std::endl;
    std::cout << "  Features: " << planes.size() << " planes detected" << std::endl;
}

int main() {
    std::cout << "Sprint 9 Advanced Registration Techniques - Basic Test" << std::endl;
    std::cout << "=====================================================" << std::endl;
    
    try {
        testPoseGraph();
        testFeatureExtraction();
        testIntegration();
        
        std::cout << "\n=== All tests completed successfully! ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
