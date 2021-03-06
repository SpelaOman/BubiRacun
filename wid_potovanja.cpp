#include <QtSql>
#include <QMessageBox>

#include "wid_potovanja.h"
#include "ui_wid_potovanja.h"
#include "potovanja.h"
#include "kodiranje.h"
#include "potninalogi.h"
#include "datum.h"

wid_potovanja::wid_potovanja(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::wid_potovanja)
{
	ui->setupUi(this);

	// disable and hide
	ui->txt_stnaloga->setEnabled(false);
	ui->txt_stnaloga->setVisible(false);

	napolni();

}

wid_potovanja::~wid_potovanja()
{
	delete ui;
}

void wid_potovanja::napolni() {

	int izbranec = 0;
	int razvrsti = 0;

	if ( ui->tbl_potovanja->selectedItems().count() > 0 ) {
		izbranec = ui->tbl_potovanja->selectedItems().takeAt(0)->row();
	}

	razvrsti = ui->tbl_potovanja->horizontalHeader()->sortIndicatorSection();

		// clear previous content
		ui->tbl_potovanja->clear();

		for (int i = 0; i <= 5; i++) {
			ui->tbl_potovanja->removeColumn(0);
		}

		QSqlQuery sql_clear;
		sql_clear.prepare("SELECT * FROM potovanja");
		sql_clear.exec();
		while (sql_clear.next()) {
			ui->tbl_potovanja->removeRow(0);
		}

		// start filling the table
		ui->tbl_potovanja->insertColumn(0);
		ui->tbl_potovanja->insertColumn(1);
		ui->tbl_potovanja->insertColumn(2);
		ui->tbl_potovanja->insertColumn(3);
		ui->tbl_potovanja->insertColumn(4);
		ui->tbl_potovanja->insertColumn(5);

		QTableWidgetItem *naslov0 = new QTableWidgetItem;
		QTableWidgetItem *naslov1 = new QTableWidgetItem;
		QTableWidgetItem *naslov2 = new QTableWidgetItem;
		QTableWidgetItem *naslov3 = new QTableWidgetItem;
		QTableWidgetItem *naslov4 = new QTableWidgetItem;
		QTableWidgetItem *naslov5 = new QTableWidgetItem;

		naslov0->setText("ID");
		naslov1->setText("Odhod");
		naslov2->setText("Prihod");
		naslov3->setText("Datum odhoda");
		naslov4->setText("Datum prihoda");
		naslov5->setText("Kilometri");

		ui->tbl_potovanja->setHorizontalHeaderItem(0, naslov0);
		ui->tbl_potovanja->setHorizontalHeaderItem(1, naslov1);
		ui->tbl_potovanja->setHorizontalHeaderItem(2, naslov2);
		ui->tbl_potovanja->setHorizontalHeaderItem(3, naslov3);
		ui->tbl_potovanja->setHorizontalHeaderItem(4, naslov4);
		ui->tbl_potovanja->setHorizontalHeaderItem(5, naslov5);

		ui->tbl_potovanja->setColumnWidth(0, 35);

		datum *delegate = new datum(this);
		ui->tbl_potovanja->setItemDelegateForColumn(3, delegate);
		ui->tbl_potovanja->setItemDelegateForColumn(4, delegate);

		QSqlQuery sql_fill;
		sql_fill.prepare("SELECT * FROM potovanja WHERE potni_nalog LIKE '" + pretvori(ui->txt_stnaloga->text()) + "'");
		sql_fill.exec();

		int row = 0;
		while (sql_fill.next()) {
			ui->tbl_potovanja->insertRow(row);
			ui->tbl_potovanja->setRowHeight(row, 20);
			int col = 0;
			int i = 0;
			QString polja[6] = {"id", "kraj_odhoda", "kraj_prihoda", "cas_odhoda", "cas_prihoda", "kilometri"};

			while (col <= 5) {

				QTableWidgetItem *celica = new QTableWidgetItem;
				if ( polja[i] == "id" ) {
					celica->setData(Qt::DisplayRole, prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()).toInt());
				}
				else if ( polja[i] == "cas_odhoda" || polja[i] == "cas_prihoda") {
					celica->setData(Qt::DisplayRole, QDateTime::fromString(prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()), "dd'.'MM'.'yyyy HH':'mm"));
				}
				else if ( polja[i] == "kilometri" ) {
					celica->setText(prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()).replace(".", ",") + " km");
				}
				else {
					celica->setText(prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()));
				}
				ui->tbl_potovanja->setItem(row, col, celica);

				col++;
				i++;

			}

			row++;

		}

	prenesi();

	ui->tbl_potovanja->selectRow(izbranec);
	ui->tbl_potovanja->sortByColumn(razvrsti, Qt::AscendingOrder);

}

void wid_potovanja::on_tbl_potovanja_doubleClicked() {

	potovanja *uredi = new potovanja;
	uredi->show();
	QObject::connect(this, SIGNAL(prenos(QString)),
			   uredi , SLOT(prejem(QString)));
	prenos(ui->tbl_potovanja->selectedItems().takeAt(0)->text());

	// receive signal to refresh table
	QObject::connect(uredi, SIGNAL(poslji(QString)),
			   this , SLOT(osvezi(QString)));

}

void wid_potovanja::on_btn_brisi_clicked() {

	QString id = ui->tbl_potovanja->selectedItems().takeAt(0)->text();

		QSqlQuery sql_brisi;
		sql_brisi.prepare("DELETE FROM potovanja WHERE id LIKE '" + id + "'");
		sql_brisi.exec();

	ui->tbl_potovanja->removeRow(ui->tbl_potovanja->selectedItems().takeAt(0)->row());
	osvezi("potovanja");

}

void wid_potovanja::osvezi(QString beseda) {

	if ( beseda == "potovanja" ) {
		napolni();
	}

}

QString wid_potovanja::pretvori(QString besedilo) {

	return kodiranje().zakodiraj(besedilo);

}

QString wid_potovanja::prevedi(QString besedilo) {

	return kodiranje().odkodiraj(besedilo);

}

void wid_potovanja::on_btn_nov_clicked() {

	potovanja *uredi = new potovanja;
	uredi->show();
	QObject::connect(this, SIGNAL(prenos(QString)),
			   uredi , SLOT(prejem(QString)));
	prenos("Nova pot" + ui->txt_stnaloga->text());

	// receive signal to refresh table
	QObject::connect(uredi, SIGNAL(poslji(QString)),
			   this , SLOT(osvezi(QString)));

}

void wid_potovanja::prejem(QString besedilo) {

	ui->txt_stnaloga->setText(besedilo);

	napolni();

}
