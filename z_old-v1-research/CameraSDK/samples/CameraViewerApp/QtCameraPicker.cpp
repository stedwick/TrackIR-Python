#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QVariant>
#include <QSignalBlocker>
#include <QSet>

#include "QtCameraPicker.h"
#include "QtCameraConnectionManager.h" 

// Specialized widget for selecting a camera

CameraPicker::CameraPicker(CameraConnectionManager* mgr, QWidget* parent)
    : QWidget(parent), camera_manager(mgr)
{
    auto* h = new QHBoxLayout(this);
    h->setContentsMargins(0,0,0,0);

    picker_label = new QLabel(tr("Camera:"), this);
    combo_box = new QComboBox(this);

    h->addWidget(picker_label);
    h->addWidget(combo_box, 1);

    refresh();

    connect(combo_box, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CameraPicker::onComboIndexChanged);

    if (camera_manager) {
        connect(camera_manager, &CameraConnectionManager::camerasChanged,
                this, &CameraPicker::onCamerasChanged);
    }

    emit camerasPresentChanged(combo_box->count() > 0);
}

void CameraPicker::setSelectedSerial(std::optional<unsigned> s)
{
    selected_serial = s;
    int targetIdx = -1;
    if (selected_serial) {
        const qulonglong want = static_cast<qulonglong>(*selected_serial);
        for (int i = 0; i < combo_box->count(); ++i) {
            if (combo_box->itemData(i).value<qulonglong>() == want) {
                targetIdx = i; break;
            }
        }
    }
    QSignalBlocker blocker(combo_box);
    combo_box->setCurrentIndex(targetIdx);
}

void CameraPicker::onComboIndexChanged(int idx)
{
    if (idx < 0 || combo_box->count() == 0) {
        selected_serial.reset();
        emit serialChanged(std::nullopt);
        return;
    }
    const auto serial = combo_box->itemData(idx).value<qulonglong>();
    selected_serial = static_cast<unsigned>(serial);
    emit serialChanged(selected_serial);
}

void CameraPicker::onCamerasChanged()
{
    const bool hadAny = (combo_box->count() > 0);
    refresh();
    const bool hasAny = (combo_box->count() > 0);

    if (hadAny != hasAny) {
        emit camerasPresentChanged(hasAny);
    }

    emit serialChanged(selected_serial);
}

void CameraPicker::refresh()
{
    if (!camera_manager) return;

    auto cams = camera_manager->GetCameras();

    QSet<qulonglong> present;
    present.reserve(cams.size());
    for (const auto& c : cams) present.insert(static_cast<qulonglong>(c->Serial()));

    for (int i = combo_box->count() - 1; i >= 0; --i) {
        const auto serial = combo_box->itemData(i).value<qulonglong>();
        if (!present.contains(serial)) {
            combo_box->removeItem(i);
        }
    }

    // Add new entries
    for (const auto& c : cams) {
        const qulonglong s = static_cast<qulonglong>(c->Serial());
        bool found = false;
        for (int i = 0; i < combo_box->count(); ++i) {
            if (combo_box->itemData(i).value<qulonglong>() == s) { found = true; break; }
        }
        if (!found) {
            const QString camName = QString::fromUtf8(c->Name());
            const QString text = QStringLiteral("%1  (%2x%3)")
                                   .arg(camName)
                                   .arg(c->PhysicalPixelWidth())
                                   .arg(c->PhysicalPixelHeight());
            combo_box->addItem(text, QVariant::fromValue<qulonglong>(s));
        }
    }

    // Preserve/choose selection
    int targetIdx = -1;
    if (selected_serial &&
        present.contains(static_cast<qulonglong>(*selected_serial))) {
        const qulonglong want = static_cast<qulonglong>(*selected_serial);
        for (int i = 0; i < combo_box->count(); ++i) {
            if (combo_box->itemData(i).value<qulonglong>() == want) { targetIdx = i; break; }
        }
    } else if (combo_box->count() > 0) {
        targetIdx = 0;
        selected_serial = static_cast<unsigned>(combo_box->itemData(0).value<qulonglong>());
    } else {
        selected_serial.reset();
    }

    QSignalBlocker blocker(combo_box);
    combo_box->setCurrentIndex(targetIdx);
}
