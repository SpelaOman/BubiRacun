#ifndef PRIJAVA_H
#define PRIJAVA_H

#include <QDialog>

#include "varnost.h"

namespace Ui {
    class prijava;
}

class prijava : public QDialog
{
    Q_OBJECT

public:
	explicit prijava(QWidget *parent = 0);
	~prijava();

private slots:
	void on_btn_izhod_clicked();
	void on_btn_brisi_clicked();
	void on_btn_prijavi_clicked();
	void keyPressEvent(QKeyEvent *event);

	QString pretvori(QString besedilo);
	QString prevedi(QString besedilo);

	// ustvari varnostno kopijo
	void varnostna_kopija();

	// ustvari tabele
	void tabela_podjetje();
	void tabela_uporabnik();
	void tabela_stranke();

	// ustvari tabele sifrantov
	void tabela_skd();
	void tabela_posta();
	void tabela_nazivi();
	void tabela_pogodbe();
	void tabela_dovoljenja();
	void tabela_status_projekta();
	void tabela_popusti();
	void tabela_viri_strank();

	// vnese podatke v tabele
	void vnesi_skd();
	void vnesi_posta();
	void vnesi_nazive();
	void vnesi_pogodbe();
	void vnesi_dovoljenja();
	void vnesi_status_projekta();
	void vnesi_popuste();
	void vnesi_vire_strank();

signals:
	void prenos1(QString besedilo);
	void prenos2(QString besedilo);
	void secret(QString pass);
	void obvestilo(QString obvestilo);

private:
    Ui::prijava *ui;
};

#endif // PRIJAVA_H
