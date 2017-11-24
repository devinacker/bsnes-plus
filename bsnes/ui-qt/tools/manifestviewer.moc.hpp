class ManifestViewerWindow : public QWidget {
	Q_OBJECT
	
public:
	ManifestViewerWindow();
	
	void synchronize();

private:
	QVBoxLayout *layout;
	QPlainTextEdit *textEdit;

};

extern ManifestViewerWindow *manifestViewerWindow;
