class RegisterEdit : public QWidget {
	Q_OBJECT

protected:
	SNES::ChipDebugger &_debugger;

protected slots:
	virtual void setupUI() = 0;
	virtual void commit() = 0;
	
public slots:
	virtual void synchronize() = 0;
	
public:
	RegisterEdit(SNES::ChipDebugger &debugger, QWidget *parent = 0) : QWidget(parent), _debugger(debugger) {}
};

class RegisterEditCPU : public RegisterEdit {
	Q_OBJECT
	
	QLineEdit *edit_pc, *edit_a, *edit_x, *edit_y, *edit_s, *edit_d, *edit_p, *edit_db;
	QCheckBox *flag_btn[9];
	
protected slots:
	void setupUI();
	void commit();

public slots:
	void synchronize();
	
public:
	RegisterEditCPU(SNES::ChipDebugger &debugger, QWidget *parent = 0)
		: RegisterEdit(debugger, parent)
	{
		setupUI();
	}
};

class RegisterEditSMP : public RegisterEdit {
	Q_OBJECT
	
	QLineEdit *edit_pc, *edit_a, *edit_x, *edit_y, *edit_s, *edit_ya, *edit_p;
	QCheckBox *flag_btn[8];
	
protected slots:
	void setupUI();
	void commit();

public slots:
	void synchronize();
	
public:
	RegisterEditSMP(QWidget *parent = 0)
		: RegisterEdit(SNES::smp, parent)
	{
		setupUI();
	}
};

class RegisterEditSFX : public RegisterEdit {
	Q_OBJECT
	
	QLineEdit *edit_r[16];
	QLineEdit *edit_sfr;
	QCheckBox *flag_btn[12];
	
protected slots:
	void setupUI();
	void commit();

public slots:
	void synchronize();
	
public:
	RegisterEditSFX(QWidget *parent = 0)
		: RegisterEdit(SNES::superfx, parent)
	{
		setupUI();
	}
};

class RegisterEditSGB : public RegisterEdit {
	Q_OBJECT
	
	QLineEdit *edit_pc, *edit_af, *edit_bc, *edit_de, *edit_hl, *edit_sp;
	QCheckBox *flag_btn[4];
	
protected slots:
	void setupUI();
	void commit();

public slots:
	void synchronize();
	
public:
	RegisterEditSGB(QWidget *parent = 0)
		: RegisterEdit(SNES::supergameboy, parent)
	{
		setupUI();
	}
};

extern RegisterEditCPU *registerEditCPU, *registerEditSA1;
extern RegisterEditSMP *registerEditSMP;
extern RegisterEditSFX *registerEditSFX;
extern RegisterEditSGB *registerEditSGB;
