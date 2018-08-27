#include "soundviewer.moc"
SoundViewerWindow *soundViewerWindow;

#include <ctgmath>

#define OCTAVE_OFFSET 7
#define NUM_OCTAVES 8

#define KEY_HEIGHT 48
#define KEY_WIDTH 12
#define KEY_BLACK_MARGIN 2
#define METER_HEIGHT 12
#define METER_MARGIN 2

const QRgb colors[] = {
	0xFFFF0000,
	0xFFFFB200,
	0xFF99FF00,
	0xFF00FF19,
	0xFF00CBFF,
	0xFF007FFF,
	0xFF3300FF,
	0xFFE500FF
};

SoundViewerWidget::SoundViewerWidget(unsigned ch) {
	this->channel = ch;
	this->volume = 0.0;
	this->panL = 0.0;
	this->panR = 0.0;
	this->note = -1;
	
	initPixmap();
	
	setMinimumWidth(NUM_OCTAVES * pixmap.width());
	setMaximumWidth(NUM_OCTAVES * pixmap.width());
	setMinimumHeight(pixmap.height() + METER_HEIGHT);
	setMaximumHeight(pixmap.height() + METER_HEIGHT);
}

void SoundViewerWidget::initPixmap() {
	// create the pixmap for a single octave on the keyboard
	pixmap = QPixmap(7 * KEY_WIDTH, KEY_HEIGHT);
	
	QPainter painter(&pixmap);
	
	painter.fillRect(pixmap.rect(), Qt::white);
	painter.setPen(Qt::black);
	// white keys
	#define key(num, pos) \
		keyRect[num] = QRect(pos * KEY_WIDTH, 0, KEY_WIDTH - 1, KEY_HEIGHT - 1);\
		painter.drawRect(keyRect[num]); \
		keyRect[num].adjust(1, 1, 0, 0);
	
	key(0, 0); key(2, 1); key(4, 2); key(5 ,3); key(7, 4); key(9, 5); key(11, 6);
	
	#undef key
	
	// black keys
	#define key(num, pos)\
		keyRect[num] = QRect(pos * KEY_WIDTH + (0.5 * KEY_WIDTH + KEY_BLACK_MARGIN), \
			0, KEY_WIDTH - 2 * KEY_BLACK_MARGIN, 0.6 * KEY_HEIGHT); \
		painter.fillRect(keyRect[num], Qt::black);
	
	key(1, 0); key(3, 1); key(6, 3); key(8, 4); key(10, 5);
	
	#undef key
}

void SoundViewerWidget::refresh() {
	// get volume/panning
	unsigned outx = abs((int8_t)SNES::dsp.read(0x9 /* SPC_DSP::v_outx */ + (this->channel << 4)));
	// this is a pretty crappy way to meter volume, but it mostly works
	this->volume = (((double)outx / 127) + 3 * this->volume) / 4;
	
	unsigned mvol_l = abs((int8_t)SNES::dsp.read(0x0c));
	unsigned mvol_r = abs((int8_t)SNES::dsp.read(0x1c));
	unsigned vvol_l = abs((int8_t)SNES::dsp.read(0x0 + (this->channel << 4)));
	unsigned vvol_r = abs((int8_t)SNES::dsp.read(0x1 + (this->channel << 4)));
	
	// voice volume is scaled down less than master volume since it usually seems to be set pretty low
	this->panL = (double)(mvol_l * vvol_l) / (128 * 32);
	this->panR = (double)(mvol_r * vvol_r) / (128 * 32);

	// get pitch/note
	if (this->volume > 0.01) {
		unsigned pitch = ((SNES::dsp.read(0x2 /* SPC_DSP::v_pitchl */ + (this->channel << 4)) << 0)
						+ (SNES::dsp.read(0x3 /* SPC_DSP::v_pitchh */ + (this->channel << 4)) << 8));
		
		double l = log2((double)pitch) - OCTAVE_OFFSET;
		this->note = (int)round(l * 12);
	} else {
		this->note = -1;
	}
	
	update();
}

void SoundViewerWidget::paintEvent(QPaintEvent *event) {
	(void)event;
	
	QPainter painter(this);
	
	painter.fillRect(rect(), Qt::transparent);
	
	QRect temp = rect();
	temp.moveTop(METER_HEIGHT);
	painter.drawTiledPixmap(temp, pixmap);
	
	// draw note
	if (this->note >= 0 && this->note < 12 * NUM_OCTAVES) {
		int octave = this->note / 12;
		int nn = this->note % 12;
		
		int adjust = 7 * KEY_WIDTH * octave;
		QRect noteRect = keyRect[nn].adjusted(adjust, METER_HEIGHT, adjust, METER_HEIGHT);
		painter.fillRect(noteRect, colors[this->channel]);
	}
	
	// draw volume
	int meterLength = rect().width() / 2;
	int left = (this->volume * this->panL) * meterLength;
	int right = (this->volume * this->panR) * meterLength;
	painter.fillRect(QRect(meterLength, 0, -left, METER_HEIGHT - METER_MARGIN), colors[this->channel]);
	painter.fillRect(QRect(meterLength, 0, right, METER_HEIGHT - METER_MARGIN), colors[this->channel]);
}

SoundViewerWindow::SoundViewerWindow() {
	setObjectName("sound-viewer");
	setWindowTitle("Sound Viewer");
	setGeometryString(&config().geometry.soundViewerWindow);
	application.windowList.append(this);
  
	layout = new QVBoxLayout;
	layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	layout->setMargin(Style::WindowMargin);
	layout->setSpacing(Style::WidgetSpacing);
	layout->setSizeConstraint(QLayout::SetFixedSize);
	setLayout(layout);
	
	for (int i = 0; i < 8; i++) {
		noteLabel[i] = new QLabel(QString("Channel %1").arg(i+1));
		layout->addWidget(noteLabel[i]);
		viewer[i] = new SoundViewerWidget(i);
		layout->addWidget(viewer[i]);
	}
}

void SoundViewerWindow::updateValues() {
	for (int i = 0; i < 8; i++) {
		viewer[i]->refresh();
	}	
	
	if (isVisible()) QTimer::singleShot(15, this, SLOT(updateValues()));
}

void SoundViewerWindow::setVisible(bool on) {
	QWidget::setVisible(on);
	if (on) updateValues();
}
