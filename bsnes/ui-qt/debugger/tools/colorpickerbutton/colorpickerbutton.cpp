#include "colorpickerbutton.moc.hpp"
#include <QColorDialog>

ColorPickerButton::ColorPickerButton(const QColor &defaultColor, QWidget *parent)
    : QPushButton(parent), m_color(defaultColor) {
    connect(this, &QPushButton::clicked, this, &ColorPickerButton::pickColor);
    updateButtonStyle();
}

QColor ColorPickerButton::color() const {
    return m_color;
}

void ColorPickerButton::setColor(const QColor &color) {
    if (color.isValid() && m_color != color) {
        m_color = color;

        if (objectName().isEmpty()) {
            setObjectName(QString("color-picker-%1").arg(reinterpret_cast<quintptr>(this)));
        }

        updateButtonStyle();
        emit colorChanged(m_color);
    }
}

void ColorPickerButton::pickColor() {
    QColor selectedColor = QColorDialog::getColor(m_color, this, "Select Color");
    if (selectedColor.isValid()) {
        setColor(selectedColor);
    }
}

void ColorPickerButton::updateButtonStyle() {
    QString style = QString(
        "#%1 { background-color: %2; border: 1px solid #999; min-width: 40px; }"
    ).arg(objectName(), m_color.name());
    
    setStyleSheet(style);
}
