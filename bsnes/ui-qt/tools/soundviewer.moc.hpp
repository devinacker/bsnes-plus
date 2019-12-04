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
	QCheckBox *channelEnable[8];
	QLabel *channelSource[8];
	QCheckBox *channelEcho[8];
	QCheckBox *channelNoise[8];
	QCheckBox *channelPitchMod[8];
	SoundViewerWidget *viewer[8];

	SoundViewerWindow();

public slots:
	void synchronize();
	void setVisible(bool);
	
private slots:
	void synchronizeDSP();
};

extern SoundViewerWindow *soundViewerWindow;
