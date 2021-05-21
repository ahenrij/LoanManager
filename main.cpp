#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include "FenPrincipale.h"
#include <cstdlib>
#include <ctime>

int main(int argc, char *argv[]){
    QApplication app(argc,argv);

    //Traduction selon la langue du terminal utilisateur
    QString locale = QLocale::system().name().section('_', 0, 0);
    QTranslator translator;
    translator.load(QString("qt_") + locale,
    QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&translator);

    /* TODO
     * Tester SET NULL a idAvaliseur
     */
    srand(time(0)); //Initialisation des nbres aléatoires
    FenPrincipale fenetre;

    if(fenetre.isConnected())
    {
        fenetre.show();
        if(fenetre.utilisateurExiste()){
            fenetre.afficherDialogueAuthentification(); //QDialog d'authentification
        }else{
            fenetre.afficherDialogueNouvelUtilisateur();
        }
    }else{
        app.quit();
        //Le processus est démarré et n'est pas arrêté
    }

    return app.exec();
}
