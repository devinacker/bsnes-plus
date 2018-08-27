class SoundViewerWidget : public QWidget {
	Q_OBJECT
	
public:
	SoundViewerWidget(unsigned ch);

	void refresh();

protected:
	void paintEvent(QPaintEvent*);
	
private:
	void initPixmap();
	
	double volume, panL, panR;
	unsigned channel;
	int note;
	
	QPixmap pixmap;
	QRect keyRect[12];
};

class SoundViewerWindow : public Window {
	Q_OBJECT

public:
	QVBoxLayout *layout;
	QLabel *noteLabel[8];
	SoundViewerWidget *viewer[8];

	SoundViewerWindow();

public slots:
	void updateValues();
	void setVisible(bool);
};

extern SoundViewerWindow *soundViewerWindow;
