#ifndef TARGET_H
#define TARGET_H

#include <QString>
#include <QVector3D>
#include <QVariantMap>
#include <QList>
#include <memory>

/**
 * @brief Base class for all registration targets
 */
class Target {
public:
    Target(const QString& id, const QVector3D& pos);
    virtual ~Target() = default;
    
    virtual QString getType() const = 0;
    virtual QVariantMap serialize() const;
    virtual bool deserialize(const QVariantMap& data);
    
    // Getters
    QString getTargetId() const { return m_targetId; }
    QVector3D getPosition() const { return m_position; }
    float getQuality() const { return m_quality; }
    bool isValid() const { return m_isValid; }
    
    // Setters
    void setPosition(const QVector3D& position) { m_position = position; }
    void setQuality(float quality) { m_quality = quality; }
    void setValid(bool valid) { m_isValid = valid; }

protected:
    QString m_targetId;
    QVector3D m_position;
    float m_quality;
    bool m_isValid;
};

/**
 * @brief Sphere target for automatic detection
 */
class SphereTarget : public Target {
public:
    SphereTarget(const QString& id, const QVector3D& pos, float radius);
    
    QString getType() const override { return "Sphere"; }
    QVariantMap serialize() const override;
    bool deserialize(const QVariantMap& data) override;
    
    float getRadius() const { return m_radius; }
    float getRMSError() const { return m_rmsError; }
    int getInlierCount() const { return m_inlierCount; }
    
    void setRadius(float radius) { m_radius = radius; }
    void setRMSError(float rms) { m_rmsError = rms; }
    void setInlierCount(int count) { m_inlierCount = count; }

private:
    float m_radius;
    float m_rmsError;
    int m_inlierCount;
};

/**
 * @brief Natural point target for manual selection
 */
class NaturalPointTarget : public Target {
public:
    NaturalPointTarget(const QString& id, const QVector3D& pos, const QString& description);
    
    QString getType() const override { return "Natural Point"; }
    QVariantMap serialize() const override;
    bool deserialize(const QVariantMap& data) override;
    
    QString getDescription() const { return m_description; }
    QVector3D getFeatureVector() const { return m_featureVector; }
    float getConfidence() const { return m_confidence; }
    
    void setDescription(const QString& desc) { m_description = desc; }
    void setFeatureVector(const QVector3D& feature) { m_featureVector = feature; }
    void setConfidence(float confidence) { m_confidence = confidence; }

private:
    QString m_description;
    QVector3D m_featureVector;
    float m_confidence;
};

/**
 * @brief Checkerboard target for calibration
 */
class CheckerboardTarget : public Target {
public:
    CheckerboardTarget(const QString& id, const QVector3D& pos, const QList<QVector3D>& corners);
    
    QString getType() const override { return "Checkerboard"; }
    QVariantMap serialize() const override;
    bool deserialize(const QVariantMap& data) override;
    
    QList<QVector3D> getCornerPoints() const { return m_cornerPoints; }
    QVector3D getNormal() const { return m_normal; }
    int getPatternWidth() const { return m_patternWidth; }
    int getPatternHeight() const { return m_patternHeight; }
    
    void setCornerPoints(const QList<QVector3D>& corners) { m_cornerPoints = corners; }
    void setNormal(const QVector3D& normal) { m_normal = normal; }
    void setPatternSize(int width, int height) { 
        m_patternWidth = width; 
        m_patternHeight = height; 
    }

private:
    QList<QVector3D> m_cornerPoints;
    QVector3D m_normal;
    int m_patternWidth;
    int m_patternHeight;
};

/**
 * @brief Target correspondence between scans
 */
struct TargetCorrespondence {
    QString targetId1;
    QString targetId2;
    QString scanId1;
    QString scanId2;
    float confidence;
    float distance;
    
    TargetCorrespondence(const QString& t1, const QString& t2, 
                        const QString& s1, const QString& s2)
        : targetId1(t1), targetId2(t2), scanId1(s1), scanId2(s2)
        , confidence(1.0f), distance(0.0f) {}
    
    bool isValid() const {
        return confidence > 0.5f && !targetId1.isEmpty() && !targetId2.isEmpty();
    }
};

#endif // TARGET_H
