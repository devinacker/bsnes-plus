#ifndef COLORPICKERBUTTON_H
#define COLORPICKERBUTTON_H

class ColorPickerButton : public QPushButton {
    Q_OBJECT

public:
    explicit ColorPickerButton(const QColor &defaultColor = Qt::white, QWidget *parent = nullptr);
    QColor color() const;
    void setColor(const QColor &color);

signals:
    void colorChanged(const QColor &color);

private slots:
    void pickColor();

private:
    void updateButtonStyle();
    QColor m_color;
};

#endif // COLORPICKERBUTTON_H
