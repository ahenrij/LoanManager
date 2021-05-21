#ifndef FENPRINCIPALE_H
#define FENPRINCIPALE_H

#include <QtWidgets>
#include <QtSql>


class FenPrincipale : public QMainWindow
{
    Q_OBJECT
public:
    explicit FenPrincipale(QWidget *parent = 0);
    bool utilisateurExiste();
    bool isConnected(){
        return estConnecte;
    }
    void afficherDialogueNouvelUtilisateur();

public slots:
    void afficherDialogueAuthentification();

signals:

protected slots:
    //Slots formulaire de login
    void user_login();
    void user_mdp_oublie();
    void quitter();
    //Slots des actions
    void afficherAcceuil();
    void afficherVersements();
    void afficherPrets();
    void afficherClients();
    void afficherAvaliseurs();
    void afficherApropos();
    void demarrerCalculatrice();
    void demarrerNotes();
    void rafraichirInfosPrets(QModelIndex index);
    void rafraichirInfosAcceuil();
    void enreg_modif_clients();
    void enreg_modif_avalis();
    void filtrerListePrets();
    void filtrerListeVersements();
    void activerSupClient();
    void activerSupAvaliseur();
    void activerSupPret();
    void activerSupVersement();
    void desactiverSuppression();
    void supprClient();
    void supprAvaliseur();
    void supprPret(QString idPret ="");
    void supprVersement();
    void viderHistorique();
    void nouveauVersement();
    void enregVersement();
    void lancerLeTimer();
    void quitterSiNonAuthentifier(); //Fermer l'application lors d'appuie sur Alt+F4 et Esc

    //************************************ Slots protected a copier (debut) ******************************
            //slots quitter dialogue
    void quitter_client();
    void quitter_pret();
    void quitter_avaliseur();
    void quitter_garantie();
    void quitter_utilisateur();
            //slots enreg
    void enreg_client();
    void enreg_pret();
    void enreg_garantie();
    void enreg_avaliseur();
    void enreg_utilisateur();
             //slots afficher dialogue
    void afficherDialogueNouveauPret();
    void afficherDialogueNouveauClient();
    void afficherDialogueNouvelAvaliseur();
    void afficherDialogueNouvelleGarantie();
    //************************************ Slots protected a copier (fin) ******************************

private:
    //Fonctions privées
    void connecter();
    QString chargerFichier(const QString nom);
    void initialiserWidgets();              //Initialise les différents composants de l'app
    void initialiserModels();               //Initialise tous les models de table
    void initialiserActions();              //Initialise toutes les QActions et les connectent a leur slots
    void creerMenu();                       //Creer le menu avec les actions
    void creerBarreOutils();                //Creer la barre d'outils
    void remplirCombosPrets();              //Remplit les combos de filtres d'affichage des prêts
    void remplirCombosVers();
    void ajouterHistorique(QString description);
    void actualiserHistorique();
    bool login_ok(){
        return authentif_ok;
    }



    //Ui
    QHBoxLayout *ui_login_form();
    QWidget *ui_acceuil();
    QWidget *ui_prets();
    QTabWidget *ui_infos_prets();
    QWidget *ui_avaliseurs();
    QWidget *ui_clients();
    QWidget *ui_garanties();
    QWidget *ui_versements();
    // ************************************Les ui a copier (debut) *******************************

    QVBoxLayout *ui_enreg_client_form();
    QVBoxLayout *ui_enreg_pret_form();
    QVBoxLayout *ui_enreg_avaliseur_form();
    QVBoxLayout *ui_enreg_garantie_form();
    QVBoxLayout *ui_enreg_versement_form();
    QVBoxLayout *ui_enreg_utilisateur_form();

    //***************************************Les ui a copier(fin)*********************************


    //Constantes
    const int FEN_W = 1000;
    const int FEN_H = 600;
    const QString APP_NAME = "LoanManager";
    const QString STYLE = "styles/style.txt";
    const QString STYLE_LOGIN_FORM = "styles/login_form_style.txt";
    const QString FILE_HISTORIQUE = "files/history.txt";

    //Constantes Tables
    const QString TABLE_USER = "Utilisateur";
    enum User {
        User_login = 1,
        User_pass = 2,
        User_text_secret = 3
    };
    const QString TABLE_CLIENT="Client";
    const QString TABLE_AVALISEUR="Avaliseur";
    const QString TABLE_GARANTIE="Garantie";
    const QString TABLE_PRET = "Pret";
    enum Pret {
        Pret_id_Client = 2,
        Pret_id_Avaliseur = 4,
        Pret_id_Garantie = 3
    };
    const QString TABLE_VERSEMENT="Versement";
    enum Versement {
        Versement_id_Client = 1,
        Versement_id_Pret = 2
    };

    //Attributs
    QSqlDatabase db;
    QLineEdit *et_user_login,
              *et_user_pass,
              *et_total_versement,
              *et_total_benefice,
              *et_montant_a_recup,
              *et_montant_reste;

    // ********************************les QLineEdit a copier (debut) ***************************
    QLineEdit *et_client_nom,
              *et_client_prenom,
              *et_client_telephone,
              *et_client_adresse,
              *et_client_profession,
              *et_client_societe,
              *et_solde,
              *et_avaliseur_nom,
              *et_avaliseur_prenom,
              *et_avaliseur_profession,
              *et_avaliseur_societe,
              *et_avaliseur_telephone,
              *et_avaliseur_adresse,
              *et_id_client,
              *et_utilisateur_login,
              *et_utilisateur_password,
              *et_utilisateur_password_confirm;


    QTextEdit *et_caracteristique_garantie,
              *et_utilisateur_secretword;
    QDoubleSpinBox *et_montant_pret;

  // ********************************les QLineEdit a copier (fin) ***************************

    //Garde le montantRestant, l'idClient et l'idPret pour chaque prêt
    qlonglong montantRestant;
    QString VERSE_ID_CLIENT,VERSE_ID_PRET;

    QDoubleSpinBox *spb_montantVersement;

    QLabel    *lab_pret_selection,
              *lab_nb_versement,
              *lab_avaliseur_pret;

    QLabel    *lab_benef_actuel,   //Les labels acceuils
              *lab_benef_total,
              *lab_titre_histo,
              *lab_nb_alerte;

    QDialog   *dialog_login,
              *dialog_nv_versement,
    // ******************************Les dialog a copier (debut) *************************
              *dialog_nouveau_client,
              *dialog_nouveau_pret,
              *dialog_nouvel_avaliseur,
              *dialog_nouvelle_garantie,
              *dialog_nouvel_utilisateur;

     // *************************Les dialog a copier (fin) *******************************


    //Les differents widgets du stacked layout (ts sauf wid_principal)
    QWidget *wid_principal,
            *wid_acceuil,
            *wid_prets,
            *wid_clients,
            *wid_avaliseurs,
            *wid_garanties,
            *wid_versements;
    QStackedLayout *lay_pager;

    //Actions
    QAction *actAcceuil,
            *actNouveauPret,
            *actNouveauVersement,
            *actNouveauClient,
            *actNouvelAvaliseur,
            *actSupClient,
            *actSupAvaliseur,
            *actSupPret,
            *actSupVersement,
            *actImport,
            *actVersements,
            *actPrets,
            *actClients,
            *actAvaliseurs,
            *actQuitter,
            *actCalculatrice,
            *actNote,
            *actAproposDeLoanManager,
            *actAboutQt;
    //Combos de filtres d'affichage
    QPushButton *btn_reactualiser;
    QComboBox *cmb_client,
              *cmb_etat_pret,
              *cmb_type_garantie,
              *cmb_mode_paie,
              *cmb_taux_interet,
              *cmb_montant_pret,
              *cmb_date_echeance,
              *cmb_avaliseur;

    QComboBox *cmb_client_vers,
              *cmb_date_vers,
    // ***********************************Les QComboBox(debut)********************************************
              *cmb_add_type_garantie,
              *cmb_enregpret_avaliseur,
              *cmb_enregpret_client,
              *cmb_enreg_etat_pret,
              *cmb_enregpret_garantie,
              *cmb_mode_paiement,
              *cmb_taux_interet_pret,
              *cmb_id_pret;

       //*************************************Les QComboBox (fin) ********************************************

        //*************************************Les QDateEdit (debut) ********************************************

    QDateEdit *date_nais_client,
              *date_echeance_pret,
              *date_pret;

        //*************************************Les QDateEdit (fin) ********************************************


    //MODELS ET VUES TABLES BDD
    QSqlTableModel *model_user,
                   *model_clients,
                   *model_avaliseurs,
                   *model_garanties;
    QSqlRelationalTableModel *model_prets,
                             *model_versements;

    QTableView *vue_clients,
               *vue_avaliseurs,
               *vue_garanties,
               *vue_prets,
               *vue_versements;

    QListWidget *list_alertes,
                *list_historique;
    QString USER_NAME, PASS;

    QTimer *timerAuthentification;
    bool authentif_ok;
    bool estConnecte;
};

#endif // FENPRINCIPALE_H
