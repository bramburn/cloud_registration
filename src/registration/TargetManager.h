#ifndef TARGETMANAGER_H
#define TARGETMANAGER_H

#include "Target.h"
#include <QObject>
#include <QMap>
#include <QList>
#include <QVariantMap>
#include <memory>

/**
 * @brief Central manager for registration targets and correspondences
 * 
 * This class manages all registration targets across multiple scans,
 * handles target correspondences, and provides quality assessment
 * functionality. It serves as the central data store for the
 * registration workflow.
 */
class TargetManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Target statistics structure
     */
    struct TargetStatistics {
        int totalTargets = 0;           ///< Total number of targets
        int sphereTargets = 0;          ///< Number of sphere targets
        int checkerboardTargets = 0;    ///< Number of checkerboard targets
        int naturalPointTargets = 0;    ///< Number of natural point targets
        int validTargets = 0;           ///< Number of valid targets
        int correspondences = 0;        ///< Number of correspondences
        float averageQuality = 0.0f;    ///< Average target quality
        
        /**
         * @brief Convert to QVariantMap for serialization
         */
        QVariantMap toVariantMap() const;
    };

public:
    /**
     * @brief Constructor
     * @param parent Parent QObject
     */
    explicit TargetManager(QObject* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~TargetManager() override = default;
    
    // Target management methods
    
    /**
     * @brief Add a target to a specific scan
     * @param scanId ID of the scan containing the target
     * @param target Target to add (ownership transferred)
     * @return true if target was added successfully
     */
    bool addTarget(const QString& scanId, std::shared_ptr<Target> target);
    
    /**
     * @brief Remove a target by ID
     * @param targetId ID of target to remove
     * @return true if target was removed
     */
    bool removeTarget(const QString& targetId);
    
    /**
     * @brief Get target by ID
     * @param targetId Target ID
     * @return Pointer to target, or nullptr if not found
     */
    std::shared_ptr<Target> getTarget(const QString& targetId) const;
    
    /**
     * @brief Get all targets for a specific scan
     * @param scanId Scan ID
     * @return List of targets in the scan
     */
    QList<std::shared_ptr<Target>> getTargetsForScan(const QString& scanId) const;
    
    /**
     * @brief Get all targets of a specific type
     * @param targetType Type name (e.g., "Sphere", "Checkerboard", "Natural Point")
     * @return List of targets of the specified type
     */
    QList<std::shared_ptr<Target>> getTargetsByType(const QString& targetType) const;
    
    /**
     * @brief Get all targets
     * @return List of all targets
     */
    QList<std::shared_ptr<Target>> getAllTargets() const;
    
    /**
     * @brief Get list of scan IDs that have targets
     * @return List of scan IDs
     */
    QStringList getScansWithTargets() const;
    
    /**
     * @brief Clear all targets for a specific scan
     * @param scanId Scan ID
     */
    void clearTargetsForScan(const QString& scanId);
    
    /**
     * @brief Clear all targets
     */
    void clearAllTargets();
    
    // Correspondence management methods
    
    /**
     * @brief Add a correspondence between two targets
     * @param correspondence Correspondence to add
     * @return true if correspondence was added successfully
     */
    bool addCorrespondence(const TargetCorrespondence& correspondence);
    
    /**
     * @brief Remove correspondence between two targets
     * @param targetId1 First target ID
     * @param targetId2 Second target ID
     * @return true if correspondence was removed
     */
    bool removeCorrespondence(const QString& targetId1, const QString& targetId2);
    
    /**
     * @brief Get all correspondences
     * @return List of all correspondences
     */
    QList<TargetCorrespondence> getAllCorrespondences() const;
    
    /**
     * @brief Get correspondences for a specific target
     * @param targetId Target ID
     * @return List of correspondences involving this target
     */
    QList<TargetCorrespondence> getCorrespondencesForTarget(const QString& targetId) const;
    
    /**
     * @brief Get correspondences between two scans
     * @param scanId1 First scan ID
     * @param scanId2 Second scan ID
     * @return List of correspondences between the scans
     */
    QList<TargetCorrespondence> getCorrespondencesBetweenScans(
        const QString& scanId1, const QString& scanId2) const;
    
    /**
     * @brief Clear all correspondences
     */
    void clearAllCorrespondences();
    
    // Quality assessment methods
    
    /**
     * @brief Validate all targets and correspondences
     * @return true if all data is valid
     */
    bool validateAllData() const;
    
    /**
     * @brief Get target statistics
     * @return Statistics about targets and correspondences
     */
    TargetStatistics getStatistics() const;
    
    /**
     * @brief Update target quality scores
     */
    void updateTargetQualities();
    
    /**
     * @brief Find potential correspondences automatically
     * @param scanId1 First scan ID
     * @param scanId2 Second scan ID
     * @param maxDistance Maximum distance for correspondence matching
     * @return List of suggested correspondences
     */
    QList<TargetCorrespondence> findPotentialCorrespondences(
        const QString& scanId1, const QString& scanId2, float maxDistance = 1.0f) const;
    
    // Serialization methods
    
    /**
     * @brief Serialize all data to QVariantMap
     * @return Serialized data
     */
    QVariantMap serialize() const;
    
    /**
     * @brief Deserialize data from QVariantMap
     * @param data Serialized data
     * @return true if deserialization was successful
     */
    bool deserialize(const QVariantMap& data);
    
    /**
     * @brief Save targets and correspondences to file
     * @param filePath File path to save to
     * @return true if save was successful
     */
    bool saveToFile(const QString& filePath) const;
    
    /**
     * @brief Load targets and correspondences from file
     * @param filePath File path to load from
     * @return true if load was successful
     */
    bool loadFromFile(const QString& filePath);

signals:
    /**
     * @brief Emitted when a target is added
     * @param scanId Scan ID
     * @param targetId Target ID
     */
    void targetAdded(const QString& scanId, const QString& targetId);
    
    /**
     * @brief Emitted when a target is removed
     * @param targetId Target ID
     */
    void targetRemoved(const QString& targetId);
    
    /**
     * @brief Emitted when a target is updated
     * @param targetId Target ID
     */
    void targetUpdated(const QString& targetId);
    
    /**
     * @brief Emitted when a correspondence is added
     * @param targetId1 First target ID
     * @param targetId2 Second target ID
     */
    void correspondenceAdded(const QString& targetId1, const QString& targetId2);
    
    /**
     * @brief Emitted when a correspondence is removed
     * @param targetId1 First target ID
     * @param targetId2 Second target ID
     */
    void correspondenceRemoved(const QString& targetId1, const QString& targetId2);
    
    /**
     * @brief Emitted when data validation fails
     * @param errorMessage Error description
     */
    void validationError(const QString& errorMessage);
    
    /**
     * @brief Emitted when statistics are updated
     * @param stats Current statistics
     */
    void statisticsUpdated(const TargetStatistics& stats);

private:
    /**
     * @brief Generate unique target ID
     * @return Unique ID string
     */
    QString generateUniqueTargetId() const;
    
    /**
     * @brief Validate correspondence
     * @param correspondence Correspondence to validate
     * @return true if correspondence is valid
     */
    bool validateCorrespondence(const TargetCorrespondence& correspondence) const;
    
    /**
     * @brief Calculate correspondence distance
     * @param correspondence Correspondence to analyze
     * @return Distance between corresponding targets
     */
    float calculateCorrespondenceDistance(const TargetCorrespondence& correspondence) const;
    
    /**
     * @brief Update correspondence quality
     * @param correspondence Correspondence to update
     */
    void updateCorrespondenceQuality(TargetCorrespondence& correspondence) const;

private:
    QMap<QString, std::shared_ptr<Target>> m_targets;           ///< All targets by ID
    QMap<QString, QStringList> m_scanTargets;                  ///< Target IDs by scan ID
    QList<TargetCorrespondence> m_correspondences;             ///< All correspondences
    
    mutable int m_nextTargetId;                                 ///< Counter for unique IDs
};

#endif // TARGETMANAGER_H
