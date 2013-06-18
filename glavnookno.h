#ifndef GLAVNOOKNO_H
#define GLAVNOOKNO_H

#include <QMainWindow>
#include <QSystemTrayIcon>

class QAction;

namespace Ui {
    class GlavnoOkno;
}

class GlavnoOkno : public QMainWindow
{
    Q_OBJECT

public:
    explicit GlavnoOkno(QWidget *parent = 0);
    ~GlavnoOkno();
    void setVisible(bool visible);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void sekundnik();
    void on_actionUporabniki_triggered();
    void on_actionPodjetje_triggered();
    void on_actionPotni_nalogi_triggered();
    void on_actionPrejeti_ra_uni_triggered();
    void on_actionStranke_triggered();
    void on_actionProjekti_triggered();
    void on_actionKuponi_triggered();
    void on_actionVizitka_triggered();
    void on_action_asovnice_triggered();
    void on_actionOpombe_pri_ra_unih_triggered();
    void on_actionKilometrina_triggered();
    void on_actionDnevnice_triggered();
    void on_actionDelavniki_triggered();
    void on_actionCena_malice_triggered();
    void on_actionPrihodek_triggered();
    void on_actionIzdatek_triggered();
    void on_actionPoslovanje_triggered();
    void varnost_id_changed();
    void on_actionNastavitve_triggered();
    void on_actionAvtomobili_triggered();
    void on_actionDopusti_bolni_ke_triggered();
    void on_actionPredra_uni_triggered();
    void on_actionPredpla_ilni_ra_uni_triggered();
    void on_actionRa_uni_triggered();
    void on_actionVrednost_DDV_triggered();
    void keyPressEvent(QKeyEvent *event);

    void on_btn_home_clicked();

    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    void podatki();
    void zacetek();
    void osnovni_pogled();

    QString pretvori(QString besedilo);
    QString prevedi(QString besedilo);

signals:
    void prenos(QString beseda);

private:
    QSystemTrayIcon *ikonca;
    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *restoreAction;
    QAction *quitAction;
    Ui::GlavnoOkno *ui;
};

#endif // GLAVNOOKNO_H
