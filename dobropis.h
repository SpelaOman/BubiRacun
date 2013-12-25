#ifndef DOBROPIS_H
#define DOBROPIS_H

#include <QDialog>

namespace Ui {
class dobropis;
}

class dobropis : public QDialog
{
	Q_OBJECT

public:
	explicit dobropis(QWidget *parent = 0);
	~dobropis();

private slots:
	void on_btn_izhod_clicked();
	void on_btn_izprazni_clicked();

	void prejem(QString besedilo);
	void napolni_racun();
	void napolni_tabelo();
	void on_tbl_storitve_itemSelectionChanged();

	QString pretvori(QString besedilo);
	QString prevedi(QString besedilo);



/*
	void on_btn_izvozi_clicked();
	void on_cb_predlagatelj_oseba_toggled();
	void on_cb_prejemnik_oseba_toggled();
	void on_txt_predlagatelj_izbira_oseba_currentIndexChanged();
	void on_txt_prejemnik_izbira_osebe_currentIndexChanged();
	void on_txt_prevoz_currentIndexChanged(int index);
	void on_txt_datum_naloga_dateChanged();
	void on_btn_sprejmi_clicked();
	void keyPressEvent(QKeyEvent *event);
	void on_txt_registrska_stevilka_currentIndexChanged();

	void on_btn_predlagatelj_uredi_clicked();
	void on_btn_prejemnik_uredi_clicked();

	void on_txt_priznana_dnevnica_toggled();
	void on_txt_zajtrk_8_12_toggled();
	void on_txt_zajtrk_12_24_toggled();

	void prejem(QString besedilo);
	void sprejmist();
	void sprejmipot();
	void izracun();
	void stevilka_racuna();
	void registrska_stevilka();

	QString pretvori_v_double(QString besedilo);
	QString pretvori_iz_double(QString besedilo);*/

signals:
/*	void poslji(QString beseda);
	void prenos(QString beseda);
	void tiskaj(QString vrsta, QString stevilke, QString format);*/

private:
	Ui::dobropis *ui;
};

#endif // DOBROPIS_H
