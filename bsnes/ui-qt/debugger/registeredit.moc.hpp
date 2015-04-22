class RegisterEdit : public QWidget {
	Q_OBJECT

protected slots:
	virtual void setupUI() = 0;
	virtual void commit() = 0;
	
public slots:
	virtual void synchronize() = 0;
	
public:
	RegisterEdit(QWidget *parent = 0) : QWidget(parent) {}
};

class RegisterEditCPU : public RegisterEdit {
	Q_OBJECT
	
	SNES::CPUcore::reg24_t &pc;
	SNES::CPUcore::reg16_t &a, &x, &y, &s, &d;
	SNES::CPUcore::flag_t &p;
	SNES::uint8 &db;
	bool &e;
	
	QLineEdit *edit_pc, *edit_a, *edit_x, *edit_y, *edit_s, *edit_d, *edit_p, *edit_db;
	QCheckBox *flag_btn[9];
	
protected slots:
	void setupUI();
	void commit();

public slots:
	void synchronize();
	
public:
	RegisterEditCPU(SNES::CPUcore::regs_t &regs, QWidget *parent = 0)
		: RegisterEdit(parent)
		, pc(regs.pc), a(regs.a), x(regs.x), y(regs.y), s(regs.s)
		, d(regs.d), db(regs.db), p(regs.p), e(regs.e)
	{
		setupUI();
	}
};

class RegisterEditSMP : public RegisterEdit {
	Q_OBJECT
	
	uint16_t &pc;
	uint8_t &a, &x, &y, &s;
	SNES::SMPcore::regya_t &ya;
	SNES::SMPcore::flag_t &p;
	
	QLineEdit *edit_pc, *edit_a, *edit_x, *edit_y, *edit_s, *edit_ya, *edit_p;
	QCheckBox *flag_btn[8];
	
protected slots:
	void setupUI();
	void commit();

public slots:
	void synchronize();
	
public:
	RegisterEditSMP(QWidget *parent = 0)
		: RegisterEdit(parent)
		, pc(SNES::smp.regs.pc), a(SNES::smp.regs.a), x(SNES::smp.regs.x), y(SNES::smp.regs.y), s(SNES::smp.regs.sp)
		, ya(SNES::smp.regs.ya), p(SNES::smp.regs.p)
	{
		setupUI();
	}
};

class RegisterEditSFX : public RegisterEdit {
	Q_OBJECT
	
	SNES::SuperFX::reg16_t (&r)[16];
	SNES::SuperFX::sfr_t &sfr;
	
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
		: RegisterEdit(parent)
		, r(SNES::superfx.regs.r), sfr(SNES::superfx.regs.sfr)
	{
		setupUI();
	}
};

extern RegisterEditCPU *registerEditCPU, *registerEditSA1;
extern RegisterEditSMP *registerEditSMP;
extern RegisterEditSFX *registerEditSFX;
