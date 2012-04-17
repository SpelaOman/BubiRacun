#include <QPalette>
#include <QFile>

#include "main.h"
#include "prijava.h"
#include "varnost.h"
#include "glavnookno.h"

int main(int argc, char *argv[])
{
	varnost app(argc, argv);

	// create style for the application
/*
	QString pot_do_stilske_datoteke = QApplication::applicationDirPath();
	QFile datoteka(pot_do_stilske_datoteke + "/style.css");

	datoteka.open(QFile::ReadOnly);
	QString StyleSheet = QLatin1String(datoteka.readAll());

	qApp->setStyleSheet(StyleSheet);
*/
	// create signal-slot connections
	GlavnoOkno *varnost_GlavnoOkno = new GlavnoOkno;
	QObject::connect(&app, SIGNAL(id_changed()), varnost_GlavnoOkno, SLOT(varnost_id_changed()));

	prijava *okno_prijava = new prijava;
	okno_prijava->show();
	return app.exec();

}
