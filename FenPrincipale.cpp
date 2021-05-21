#include "FenPrincipale.h"


FenPrincipale::FenPrincipale(QWidget *parent) : QMainWindow(parent)
{    
    estConnecte = true;
    connecter();
    initialiserModels();
    initialiserWidgets();
    initialiserActions();
    creerMenu();
    creerBarreOutils();
    setWindowIcon(QIcon("images/ic_app.png"));
    setCentralWidget(wid_principal);
    setStyleSheet(chargerFichier(STYLE));
    setMinimumSize(FEN_W,FEN_H);
    USER_NAME = "";
    PASS ="";

    timerAuthentification = new QTimer(this);
    timerAuthentification->setSingleShot(true);
    timerAuthentification->setInterval(5000);
    connect(timerAuthentification, SIGNAL(timeout()), this, SLOT(afficherDialogueAuthentification()));
    connect(qApp,SIGNAL(applicationStateChanged(Qt::ApplicationState)),this,SLOT(lancerLeTimer()));

}

void FenPrincipale::connecter(){
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("gestion_prets.db");
    if(!db.open() || db.databaseName()!="gestion_prets.db"){
        QMessageBox::critical(this,"Erreur connexion","Impossible d'accéder à la base de données.\nErreur : "+
                              db.lastError().text());
        estConnecte =false;
        close();
    }
}

void FenPrincipale::initialiserWidgets(){

    wid_principal  = new QWidget();
    lay_pager     = new QStackedLayout();
    wid_principal->setLayout(lay_pager);

//    //Affecter aux widgets les différents ui correspondants
    wid_clients    = ui_clients();
    wid_avaliseurs = ui_avaliseurs();
    wid_versements = ui_versements();
    wid_prets      = ui_prets();
    wid_acceuil    = ui_acceuil();

    lay_pager->addWidget(wid_acceuil);
    lay_pager->addWidget(wid_prets);
    lay_pager->addWidget(wid_clients);
    lay_pager->addWidget(wid_avaliseurs);
    lay_pager->addWidget(wid_versements);

    lay_pager->setCurrentWidget(wid_acceuil);

}

void FenPrincipale::initialiserModels(){
    //Initialiser les vues
    vue_prets = new QTableView();
    vue_prets->setAlternatingRowColors(true);
    vue_avaliseurs = new QTableView();
    vue_avaliseurs->setAlternatingRowColors(true);
    vue_clients = new QTableView();
    vue_clients->setAlternatingRowColors(true);
    vue_garanties = new QTableView();
    vue_versements = new QTableView();
    vue_versements->setAlternatingRowColors(true);

    //Initialiser les models et les affecter aux vues
    model_user = new QSqlTableModel();
    model_user->setTable(TABLE_USER);

    //MODELE CLIENT
    model_clients = new QSqlTableModel();
    model_clients->setTable(TABLE_CLIENT);
    model_clients->select();
    vue_clients->setModel(model_clients);
    for(int col=1; col<model_clients->columnCount();col++) vue_clients->setColumnWidth(col,189);
    vue_clients->setItemDelegate(new QSqlRelationalDelegate(this));
    vue_clients->setSelectionBehavior(QAbstractItemView::SelectRows);
    vue_clients->hideColumn(0);

    //MODELE AVALISEURS
    model_avaliseurs = new QSqlTableModel();
    model_avaliseurs->setTable(TABLE_AVALISEUR);
    model_avaliseurs->select();
    vue_avaliseurs->setModel(model_avaliseurs);
    for(int col=1; col<model_avaliseurs->columnCount();col++) vue_avaliseurs->setColumnWidth(col,210);
    vue_avaliseurs->setSelectionBehavior(QAbstractItemView::SelectRows);
    vue_avaliseurs->setItemDelegate(new QSqlRelationalDelegate(this));

    //MODELE GARANTIES
    model_garanties = new QSqlTableModel();
    model_garanties->setTable(TABLE_GARANTIE);
    model_garanties->select();
    vue_garanties->setModel(model_garanties);
    for(int col=1; col<model_garanties->columnCount();col++) vue_garanties->setColumnWidth(col,189);

    //MODELE PRETS
    model_prets = new QSqlRelationalTableModel();
    model_prets->setTable(TABLE_PRET);
    model_prets->setJoinMode(QSqlRelationalTableModel::LeftJoin);
    model_prets->setRelation(Pret_id_Client, QSqlRelation(TABLE_CLIENT,"idClient","NomCli, Prenoms, Profession"));
    model_prets->setRelation(Pret_id_Garantie, QSqlRelation(TABLE_GARANTIE,"idGarantie","type, Caracteristiques"));
    model_prets->setHeaderData(1,Qt::Horizontal,"Etat du prêt");
    model_prets->setHeaderData(2,Qt::Horizontal,"Nom Client");
    model_prets->setHeaderData(5,Qt::Horizontal,"Type Garantie");
    model_prets->setHeaderData(6,Qt::Horizontal,"Caractéristiques");
    model_prets->setHeaderData(8,Qt::Horizontal,"Date du prêt");
    model_prets->select();
    vue_prets->setModel(model_prets);
    for(int col=1; col<model_prets->columnCount();col++) vue_prets->setColumnWidth(col,150);
//    vue_prets->hideColumn(0);
    vue_prets->hideColumn(7);
    vue_prets->setEditTriggers(QAbstractItemView::NoEditTriggers);
    vue_prets->setSelectionBehavior(QAbstractItemView::SelectRows);


    //MODELE VERSEMENT
    model_versements = new QSqlRelationalTableModel();
    model_versements->setTable(TABLE_VERSEMENT);
    model_versements->setJoinMode(QSqlRelationalTableModel::LeftJoin);
    model_versements->setRelation(1, QSqlRelation(TABLE_CLIENT,"idClient","NomCli AS \"Nom Client\", Prenoms"));
    model_versements->setRelation(2, QSqlRelation(TABLE_PRET,"idPret","datePret AS \"Date Prêt\", Modalite"));
    model_versements->select();
    vue_versements->setModel(model_versements);
    for(int col=0; col<model_versements->columnCount();col++) vue_versements->setColumnWidth(col,200);
    vue_versements->setEditTriggers(QAbstractItemView::NoEditTriggers);
    vue_versements->setSelectionBehavior(QAbstractItemView::SelectRows);


    //Connexions
    connect(vue_prets,SIGNAL(clicked(QModelIndex)),this,SLOT(rafraichirInfosPrets(QModelIndex)));
    connect(vue_clients,SIGNAL(clicked(QModelIndex)),this,SLOT(activerSupClient()));
    connect(vue_avaliseurs,SIGNAL(clicked(QModelIndex)),this,SLOT(activerSupAvaliseur()));
    connect(vue_prets,SIGNAL(clicked(QModelIndex)),this,SLOT(activerSupPret()));
    connect(vue_versements,SIGNAL(clicked(QModelIndex)),this,SLOT(activerSupVersement()));
}

void FenPrincipale::initialiserActions(){
    actAcceuil          = new QAction(tr("Acceuil"),this);
    actAcceuil->setIcon(QIcon("images/acceuil.png"));

    actNouveauPret      = new QAction(tr("Nouveau prêt"),this);
    actNouveauPret->setIcon(QIcon("images/nv_pret.png"));

    actNouveauVersement = new QAction(tr("Nouveau versement"),this);
    actNouveauVersement->setIcon(QIcon("images/nv_vers.png"));
    actNouveauVersement->setEnabled(false);

    actNouveauClient    = new QAction(tr("Nouveau client"),this);
    actNouveauClient->setIcon(QIcon("images/nv_client.png"));

    actNouvelAvaliseur  = new QAction(tr("Nouvel Avaliseur"),this);
    actNouvelAvaliseur->setIcon(QIcon("images/nv_avaliseur.png"));

    actSupClient        = new QAction(tr("Supprimer client"),this);
    actSupClient->setIcon(QIcon("images/sup_client.png"));
    actSupClient->setEnabled(false);

    actSupAvaliseur     = new QAction(tr("Supprimer avaliseur"),this);
    actSupAvaliseur->setIcon(QIcon("images/sup_avaliseur.png"));
    actSupAvaliseur->setEnabled(false);

    actSupPret          = new QAction(tr("Supprimer prêt"),this);
    actSupPret->setIcon(QIcon("images/sup_pret.png"));
    actSupPret->setEnabled(false);

    actSupVersement          = new QAction(tr("Supprimer versement"),this);
    actSupVersement->setIcon(QIcon("images/sup_versement.png"));
    actSupVersement->setEnabled(false);

    actImport           = new QAction(tr("Importer..."),this);
    actImport->setEnabled(false);

    actVersements       = new QAction(tr("Versements"),this);
    actVersements->setIcon(QIcon("images/versements.png"));

    actPrets            = new QAction(tr("Prêts"),this);
    actPrets->setIcon(QIcon("images/prets.png"));

    actClients          = new QAction(tr("Clients"),this);
    actClients->setIcon(QIcon("images/client.png"));

    actAvaliseurs       = new QAction(tr("Avaliseurs"),this);
    actAvaliseurs->setIcon(QIcon("images/avaliseur.png"));

    actCalculatrice     = new QAction(tr("Calculatrice"),this);
    actCalculatrice->setIcon(QIcon("images/calc.png"));

    actNote             = new QAction(tr("Notes"),this);
    actNote->setIcon(QIcon("images/notes.png"));

    actAproposDeLoanManager = new QAction(tr("A propos de LoanManager"),this);

    actAboutQt          = new QAction(tr("A propos de Qt"),this);
    actAboutQt->setIcon(QIcon("images/qt.png"));

    actQuitter          = new QAction(tr("Quitter"),this);
    actQuitter->setIcon(QIcon("images/quitter.png"));
    actQuitter->setShortcut(QKeySequence("Ctrl+Q"));

    //Connexions
    connect(actAcceuil,SIGNAL(triggered(bool)),this,SLOT(afficherAcceuil()));
    connect(actNouveauVersement,SIGNAL(triggered(bool)),this,SLOT(nouveauVersement()));
    connect(actNouveauPret,SIGNAL(triggered(bool)),this,SLOT(afficherDialogueNouveauPret()));
    connect(actNouveauClient,SIGNAL(triggered(bool)),this,SLOT(afficherDialogueNouveauClient()));
    connect(actNouvelAvaliseur,SIGNAL(triggered(bool)),this,SLOT(afficherDialogueNouvelAvaliseur()));
    connect(actVersements,SIGNAL(triggered(bool)),this,SLOT(afficherVersements()));
    connect(actPrets,SIGNAL(triggered(bool)),this,SLOT(afficherPrets()));
    connect(actClients,SIGNAL(triggered(bool)),this,SLOT(afficherClients()));
    connect(actAvaliseurs,SIGNAL(triggered(bool)),this,SLOT(afficherAvaliseurs()));
    connect(actSupClient,SIGNAL(triggered(bool)),this,SLOT(supprClient()));
    connect(actSupAvaliseur,SIGNAL(triggered(bool)),this,SLOT(supprAvaliseur()));
    connect(actSupPret,SIGNAL(triggered(bool)),this,SLOT(supprPret()));
    connect(actSupVersement,SIGNAL(triggered(bool)),this,SLOT(supprVersement()));
    connect(actCalculatrice,SIGNAL(triggered(bool)),this,SLOT(demarrerCalculatrice()));
    connect(actNote,SIGNAL(triggered(bool)),this,SLOT(demarrerNotes()));
    connect(actQuitter,SIGNAL(triggered(bool)),this,SLOT(quitter()));
    connect(actAproposDeLoanManager,SIGNAL(triggered(bool)),this,SLOT(afficherApropos()));
    connect(actAboutQt,SIGNAL(triggered(bool)),qApp,SLOT(aboutQt()));
}

void FenPrincipale::creerMenu(){
    QMenu *menuFichier  = menuBar()->addMenu(tr("&Fichier"));
    menuFichier->addAction(actAcceuil);
    menuFichier->addAction(actNouveauPret);
    menuFichier->addAction(actNouveauVersement);
    menuFichier->addAction(actNouveauClient);
    menuFichier->addAction(actNouvelAvaliseur);
    menuFichier->addAction(actImport);
    menuFichier->addAction(actQuitter);

    QMenu *menuAffichage = menuBar()->addMenu(tr("&Affichage"));
    menuAffichage->addAction(actPrets);
    menuAffichage->addAction(actVersements);
    menuAffichage->addAction(actClients);
    menuAffichage->addAction(actAvaliseurs);

    QMenu *menuOutils = menuBar()->addMenu(tr("&Outils"));
    menuOutils->addAction(actCalculatrice);
    menuOutils->addAction(actNote);

    QMenu *menuApropos = menuBar()->addMenu(tr("&Aide"));
    menuApropos->addAction(actAproposDeLoanManager);
    menuApropos->addAction(actAboutQt);
}

void FenPrincipale::creerBarreOutils()
{
   QToolBar *toolBar = addToolBar("Opérations de prêts");
   toolBar->addAction(actAcceuil);
   toolBar->addAction(actNouveauPret);
   toolBar->addAction(actSupPret);
   toolBar->addAction(actPrets);
   toolBar->addSeparator();   
   toolBar->addAction(actNouveauVersement);
   toolBar->addAction(actSupVersement);
   toolBar->addAction(actVersements);
   toolBar->addSeparator();
   toolBar->addAction(actNouveauClient);
   toolBar->addAction(actSupClient);
   toolBar->addAction(actClients);
   toolBar->addSeparator();
   toolBar->addAction(actNouvelAvaliseur);
   toolBar->addAction(actSupAvaliseur);
   toolBar->addAction(actAvaliseurs);
   toolBar->addSeparator();
   toolBar->addAction(actNote);
   toolBar->addAction(actCalculatrice);
}

void FenPrincipale::afficherDialogueAuthentification(){

    authentif_ok = false;
    PASS = "";
    dialog_login = new QDialog(this,Qt::WindowTitleHint|Qt::WindowSystemMenuHint);
    dialog_login->setLayout(ui_login_form());
    dialog_login->setFixedSize(420,200);
    dialog_login->setModal(true);
    dialog_login->setStyleSheet(chargerFichier(STYLE_LOGIN_FORM));
    dialog_login->setWindowTitle("Authentification");
    dialog_login->show();
    connect(dialog_login,SIGNAL(finished(int)),this,SLOT(quitterSiNonAuthentifier()));
}

void FenPrincipale::quitterSiNonAuthentifier(){
    if(PASS==""){
        qApp->closeAllWindows();
        qApp->quit();
    }
}

QHBoxLayout* FenPrincipale::ui_login_form(){

    QHBoxLayout *lay_dialog = new QHBoxLayout();
    QGridLayout *lay_main = new QGridLayout();
    QLabel      *image_user = new QLabel;

    QLabel      *lab_id = new QLabel("Identifiant");
    QLabel      *lab_mdp = new QLabel("Mot de passe");
    QPushButton *btn_focus = new QPushButton();
    QPushButton *btn_mdp_oublie = new QPushButton("Mot de passe oublié ?");
    QPushButton *btn_login = new QPushButton("Login");
    QPushButton *btn_login_quit = new QPushButton("Quitter");

    et_user_login = new QLineEdit;
    if(USER_NAME!=""){
        et_user_login->setText(USER_NAME);
        et_user_pass->setFocus();
    }
    et_user_pass  = new QLineEdit;

    //Transformations

    //Avatar aléatoire
    image_user->setPixmap(QPixmap(QDir::currentPath()+"/images/avatar"+QString::number(rand() % 7)+".png"));
    image_user->setScaledContents(true);
    image_user->setFixedSize(150,150);
    et_user_pass->setEchoMode(QLineEdit::Password);
    btn_focus->setProperty("btnLien",true);
    btn_mdp_oublie->setProperty("btnLien",true);

    lay_main->addWidget(lab_id,0,0,1,4);
    lay_main->addWidget(et_user_login,1,0,1,7);
    lay_main->addWidget(lab_mdp,2,0,1,4);
    lay_main->addWidget(et_user_pass,3,0,1,7);
    lay_main->addWidget(btn_focus);             //Prend le focus comme étant le premier btn
    lay_main->addWidget(btn_mdp_oublie,4,0,1,7);
    lay_main->addWidget(btn_login,5,1,1,3);
    lay_main->addWidget(btn_login_quit,5,4,1,3);


    lay_dialog->addWidget(image_user);
    lay_dialog->addSpacing(20);
    lay_dialog->addLayout(lay_main);

    //Connexions
    connect(et_user_login,SIGNAL(returnPressed()),this,SLOT(user_login()));
    connect(et_user_pass,SIGNAL(returnPressed()),this,SLOT(user_login()));
    connect(btn_login_quit,SIGNAL(clicked(bool)),this,SLOT(quitter()));
    connect(btn_login,SIGNAL(clicked(bool)),this,SLOT(user_login()));
    connect(btn_mdp_oublie,SIGNAL(clicked(bool)),this,SLOT(user_mdp_oublie()));
    return lay_dialog;
}

void FenPrincipale::user_login(){

    if(et_user_login->text().isEmpty()){
        QMessageBox::warning(this,"Authentification","Veuillez renseigner votre <b>identifiant</b> de compte");
        et_user_login->setFocus();
        return;
    }
    if(et_user_pass->text().isEmpty()){
        QMessageBox::warning(this,"Authentification","Veuillez renseigner votre <b>mot de passe</b> de compte");
        et_user_pass->setFocus();
        return;
    }

    model_user->setFilter("login='"+et_user_login->text()+"'");
    model_user->select();

    if(model_user->rowCount()!=1){  //Le résultat doit exiter et être unique
         QMessageBox::critical(this,"Authentification","Cet utilisateur n'existe pas");
         et_user_login->setFocus();
         return;
    }
    if(model_user->record(0).value(User_pass).toString() != et_user_pass->text()){ //Mot de passe différent
        QMessageBox::critical(this,"Authentification","Mot de passe incorrect !");
        et_user_pass->setFocus();
        return;
    }


    //Enregistrement des infos pr la session
    USER_NAME = et_user_login->text();
    PASS      = et_user_pass->text();
    authentif_ok = true;
    QMessageBox::information(this,"Authentification","Bienvenue <b>"+et_user_login->text()+" !</b>");
    dialog_login->close();
}

void FenPrincipale::user_mdp_oublie(){

    if(et_user_login->text().isEmpty()){
        QMessageBox::warning(this,"Authentification","Veuillez renseigner votre <b>identifiant</b> de compte");
        return;
    }

    model_user->setFilter("login='"+et_user_login->text()+"'");
    model_user->select();
    if(model_user->rowCount()!=1){  //Le résultat doit exiter et être unique
         QMessageBox::critical(this,"Authentification","Cet utilisateur n'existe pas");
         return;
    }

    dialog_login->hide();
    //On récupère le texte secret
    bool ok = false;
    QString texte_secret = QInputDialog::getText(this,"Mot de passe oublié",
                                                 "Identifiant : <b>"+et_user_login->text()+
                                                 "</b> | Quel est votre texte secret ?",QLineEdit::Normal,
                                                 QString(),&ok);
    //Texte non vide et ok clické
    if(ok && !texte_secret.isEmpty()){


        model_user->setFilter("login='"+et_user_login->text()+"' AND texteSecret LIKE '"+texte_secret+"'");
        model_user->select();

        //Le résultat doit exiter et être unique
        if(model_user->rowCount()!=1){
             QMessageBox::critical(this,"Authentification","Ce texte secret est incorrect");
             dialog_login->show();
             return;
        }
        //Ici, les informations sont corrects
        //Demander un nouveau mot de passe
        ok = false;
        QString new_pass =
                QInputDialog::getText(this,"Mot de passe oublié",
                                                 "Identifiant : <b>"+et_user_login->text()+"</b> | "
                                                 "Entrez votre nouveau mot de passe :",QLineEdit::Password,
                                                 QString(),&ok);
        if(ok){
            if(new_pass.isEmpty()){
                QMessageBox::critical(this,"Mot de passe oublié","Nouveau mot de passe vide !\n"
                                      "Aucune modification effectuée.");
            }else{

                //Le nouveau mot de passe est correctement spéficié

                model_user->setFilter("");
                model_user->select();
                for(int row=0; row<model_user->rowCount();row++){
                    if(model_user->record(row).value(User_text_secret).toString() == texte_secret){
                        QSqlRecord record = model_user->record(row);
                        record.setValue(User_pass,new_pass);
                        model_user->setRecord(row,record);
                        break;
                    }
                }
                model_user->submitAll();
                QMessageBox::information(this,"Mot de passe oublié","Modification effectuée avec succès !");
                dialog_login->show();
                return;
            }
        }else{
            QMessageBox::critical(this,"Mot de passe oublié","Aucune modification effectuée !");
        }

    }else{
        if(texte_secret.isEmpty()){
            QMessageBox::warning(this,"Mot de passe oublié","Vous avez entrer un texte vide");
        }
    }
    dialog_login->show();
    return;
}

QWidget* FenPrincipale::ui_prets(){

    const int LARG_COMBO_Attrib = 150;
    const int LARG_COMBO_Table  = 200;

    QWidget *widget = new QWidget();
    QVBoxLayout *lay_main = new QVBoxLayout();
    QLabel *panel = new QLabel("<h1>Liste des prêts</h1>");
    panel->setFixedHeight(80);

    //Creer les combos de filte
    QHBoxLayout *lay_filtres = new QHBoxLayout();

    QFormLayout *lay_filtre1 = new QFormLayout();
    cmb_etat_pret     = new QComboBox();
    cmb_etat_pret->addItem("Tous","%");
    cmb_etat_pret->addItem("Actif");
    cmb_etat_pret->addItem("Soldé");
    cmb_etat_pret->setFixedWidth(LARG_COMBO_Attrib);

    cmb_type_garantie = new QComboBox();
    cmb_type_garantie->addItem("Tous","%");
    cmb_type_garantie->addItem("Gage");
    cmb_type_garantie->addItem("Nantissement");
    cmb_type_garantie->addItem("Hypothèque");
    cmb_type_garantie->setFixedWidth(LARG_COMBO_Attrib);

    cmb_mode_paie     = new QComboBox();
    cmb_mode_paie->addItem("Tous","%");
    cmb_mode_paie->addItem("Mensuel");
    cmb_mode_paie->addItem("Trimestriel");
    cmb_mode_paie->addItem("Semestriel");
    cmb_mode_paie->addItem("Annuel");
    cmb_mode_paie->addItem("Cash");
    cmb_mode_paie->setFixedWidth(LARG_COMBO_Attrib);


    lay_filtre1->addRow("Etat ",cmb_etat_pret);
    lay_filtre1->addRow("Type garantie ",cmb_type_garantie);
    lay_filtre1->addRow("Modalité  ",cmb_mode_paie);

    QFormLayout *lay_filtre2 = new QFormLayout();

    cmb_taux_interet  = new QComboBox();
    cmb_taux_interet->setFixedWidth(LARG_COMBO_Attrib);
    cmb_taux_interet->addItem("Tous","%");

    cmb_montant_pret  = new QComboBox();
    cmb_montant_pret->addItem("Tous","%");
    cmb_montant_pret->setFixedWidth(LARG_COMBO_Attrib);

    cmb_date_echeance = new QComboBox();
    cmb_date_echeance->addItem("Tous","%");
    cmb_date_echeance->setFixedWidth(LARG_COMBO_Attrib);

    lay_filtre2->addRow("Taux d'intérêt : ",cmb_taux_interet);
    lay_filtre2->addRow("Montant : ",cmb_montant_pret);
    lay_filtre2->addRow("Date échéance : ",cmb_date_echeance);

    QFormLayout *lay_filtre3 = new QFormLayout();
    btn_reactualiser  = new QPushButton("Actualiser");
    btn_reactualiser->setFixedWidth(150);
    btn_reactualiser->setIcon(QIcon("images/actualiser.png"));
    cmb_client        = new QComboBox();
    cmb_client->addItem("Tous","%");
    cmb_client->setFixedWidth(LARG_COMBO_Table);
    cmb_avaliseur     = new QComboBox();
    cmb_avaliseur->addItem("Tous","%");
    cmb_avaliseur->setFixedWidth(LARG_COMBO_Table);

    lay_filtre3->addRow("Client : ",cmb_client);
    lay_filtre3->addRow("Avaliseur : ",cmb_avaliseur);

    remplirCombosPrets(); //Remplit les combos

    lay_filtres->addLayout(lay_filtre1);
    lay_filtres->addLayout(lay_filtre2);
    lay_filtres->addLayout(lay_filtre3);


    QTabWidget *tab_infos_prets = ui_infos_prets();
    tab_infos_prets->setFixedHeight(120);

    lay_main->addWidget(panel);
    lay_main->addWidget(btn_reactualiser);
    lay_main->addLayout(lay_filtres);
    lay_main->addWidget(vue_prets);
    lay_main->addWidget(tab_infos_prets);

    widget->setLayout(lay_main);

    //Connexion des signaux aux slots
    connect(btn_reactualiser,SIGNAL(clicked(bool)),this,SLOT(filtrerListePrets()));

    return widget;
}

QWidget* FenPrincipale::ui_acceuil(){

    QWidget *widget = new QWidget();
    QVBoxLayout *lay_acceuil = new QVBoxLayout();

    //Initialisation des widgets
    QLabel *lab_titre = new QLabel("<h1>Gestionnaire de prêts</h1>");
    lab_titre->setAlignment(Qt::AlignCenter);
    lab_titre->setFixedHeight(100);

    lab_titre_histo = new QLabel("Dernières opérations effectuées");
    lab_titre_histo->setObjectName("historique");
    lab_titre_histo->setFixedWidth(400);

    QPushButton *btn_vider_historique = new QPushButton("Vider historique");
    btn_vider_historique->setProperty("btnLien",true);
    btn_vider_historique->setFixedWidth(130);
    connect(btn_vider_historique,SIGNAL(clicked(bool)),this,SLOT(viderHistorique()));

    list_historique = new QListWidget();
    list_historique->setFixedWidth(400);

    QLabel *image_alerte  = new QLabel();
    image_alerte->setPixmap(QPixmap("images/alertes.png"));
    image_alerte->setScaledContents(true);
    image_alerte->setFixedSize(70,70);
    lab_nb_alerte = new QLabel();
    list_alertes = new QListWidget();


    QHBoxLayout *lay_contenu = new QHBoxLayout();
    QVBoxLayout *lay_bilan_alerte = new QVBoxLayout();
    QFormLayout *lay_bilan = new QFormLayout();
    QVBoxLayout *lay_historique = new QVBoxLayout();


    lab_benef_actuel = new QLabel();
    lab_benef_total  = new QLabel();
    lab_benef_actuel->setProperty("bilan",true);
    lab_benef_total->setProperty("bilan",true);

    lay_bilan->addRow("",new QLabel());
    lay_bilan->addRow("<h3>Bénéfice actuel : </h3>",lab_benef_actuel);
    lay_bilan->addRow("<h3>Bénéfice total  : </h3>",lab_benef_total);

    QHBoxLayout *lay_alerte = new QHBoxLayout();
    lay_alerte->addWidget(image_alerte);
    lay_alerte->addWidget(lab_nb_alerte);

    QGroupBox *groupBilan = new QGroupBox("Bilan général");
    groupBilan->setLayout(lay_bilan);

    lay_bilan_alerte->addWidget(groupBilan);
    lay_bilan_alerte->addSpacing(20);
    lay_bilan_alerte->addLayout(lay_alerte);
    lay_bilan_alerte->addWidget(list_alertes);

    lay_historique->addWidget(lab_titre_histo);
    lay_historique->addSpacing(5);
    lay_historique->addWidget(btn_vider_historique);
    lay_historique->addWidget(list_historique);

    //On ajoute un layout a droite pour les historiques
    lay_contenu->addLayout(lay_bilan_alerte);
    lay_contenu->addLayout(lay_historique);

    lay_acceuil->addWidget(lab_titre);
    lay_acceuil->addLayout(lay_contenu);
    widget->setLayout(lay_acceuil);

    rafraichirInfosAcceuil();   //Met a jour les infos de l'acceuil

    return widget;
}

QWidget* FenPrincipale::ui_clients(){
    QWidget *widget = new QWidget();
    QVBoxLayout *lay_clients = new QVBoxLayout();

    QLabel *lab_titre = new QLabel("<h1>Liste des Clients</h1>");
    lab_titre->setFixedHeight(80);
    QPushButton *btn_enreg_modif = new QPushButton("Enregistrer modifications");
    btn_enreg_modif->setIcon(QIcon("images/save.png"));
    btn_enreg_modif->setProperty("btnBas",true);

    QPushButton *btn_nv_client   = new QPushButton("Nouveau Client");
    btn_nv_client->setProperty("btnLien",true);
    btn_nv_client->setFixedSize(150,30);

    QHBoxLayout *en_tete = new QHBoxLayout();
    en_tete->addWidget(lab_titre);
    en_tete->addWidget(btn_nv_client);


    QGridLayout *lay_bas = new QGridLayout();
    lay_bas->addWidget(new QWidget(),0,0,1,5);
    lay_bas->addWidget(new QWidget(),0,6,1,5);
    lay_bas->addWidget(new QWidget(),0,12,1,2);
    lay_bas->addWidget(btn_enreg_modif,0,15,1,2);

    lay_clients->addLayout(en_tete);
    lay_clients->addWidget(vue_clients);
    lay_clients->addLayout(lay_bas);
    widget->setLayout(lay_clients);

    //Connecter les boutons aux slots
    connect(btn_enreg_modif,SIGNAL(clicked(bool)),this,SLOT(enreg_modif_clients()));
    connect(btn_nv_client,SIGNAL(clicked(bool)),this,SLOT(afficherDialogueNouveauClient()));

    return widget;
}

QWidget* FenPrincipale::ui_avaliseurs(){
    QWidget *widget = new QWidget();
    QVBoxLayout *lay_avalis = new QVBoxLayout();

    QLabel *lab_titre = new QLabel("<h1>Liste des Avaliseurs</h1>");
    lab_titre->setFixedHeight(80);

    QPushButton *btn_enreg_modif = new QPushButton("Enregistrer modifications");
    btn_enreg_modif->setIcon(QIcon("images/save.png"));
    btn_enreg_modif->setProperty("btnBas",true);

    QPushButton *btn_nv_avalis   = new QPushButton("Nouvel Avaliseur");
    btn_nv_avalis->setProperty("btnLien",true);
    btn_nv_avalis->setFixedSize(150,30);

    QHBoxLayout *en_tete = new QHBoxLayout();
    en_tete->addWidget(lab_titre);
    en_tete->addWidget(btn_nv_avalis);


    QGridLayout *lay_bas = new QGridLayout();
    lay_bas->addWidget(new QWidget(),0,0,1,5);
    lay_bas->addWidget(new QWidget(),0,6,1,5);
    lay_bas->addWidget(new QWidget(),0,12,1,2);
    lay_bas->addWidget(btn_enreg_modif,0,15,1,2);

    lay_avalis->addLayout(en_tete);
    lay_avalis->addWidget(vue_avaliseurs);
    lay_avalis->addLayout(lay_bas);
    widget->setLayout(lay_avalis);

    //Connecter les boutons aux slots
    connect(btn_enreg_modif,SIGNAL(clicked(bool)),this,SLOT(enreg_modif_avalis()));
    connect(btn_nv_avalis,SIGNAL(clicked(bool)),this,SLOT(afficherDialogueNouvelAvaliseur()));

    return widget;
}

QWidget* FenPrincipale::ui_versements(){
    QWidget *widget = new QWidget();
    QVBoxLayout *lay_versements = new QVBoxLayout();

    QLabel *lab_titre = new QLabel("<h1>Liste des Versements</h1>");
    lab_titre->setFixedHeight(80);

    QPushButton *btn_actualiser_vers= new QPushButton("Actualiser");
    btn_actualiser_vers->setIcon(QIcon("images/actualiser.png"));

    cmb_client_vers = new QComboBox();
    cmb_client_vers->addItem("Tous");

    cmb_date_vers = new QComboBox();
    cmb_date_vers->addItem("Tous");

    QGridLayout *lay_filtre = new QGridLayout();
    lay_filtre->addWidget(new QLabel("Client "),0,0,1,1);
    lay_filtre->addWidget(cmb_client_vers,0,1,1,3);
    lay_filtre->addWidget(new QLabel("Date Versement"),1,0,1,1);
    lay_filtre->addWidget(cmb_date_vers,1,1,1,3);
    lay_filtre->addWidget(btn_actualiser_vers,1,4,1,1);
    lay_filtre->addWidget(new QLabel(),1,5,1,5);

    remplirCombosVers();

    lay_versements->addWidget(lab_titre);
    lay_versements->addLayout(lay_filtre);
    lay_versements->addWidget(vue_versements);
    widget->setLayout(lay_versements);

    //Connecter les boutons aux slots
    connect(btn_actualiser_vers,SIGNAL(clicked(bool)),this,SLOT(filtrerListeVersements()));

    return widget;
}

QTabWidget* FenPrincipale::ui_infos_prets(){
    QTabWidget *widget_tab = new QTabWidget();

    lab_avaliseur_pret = new QLabel();
    lab_pret_selection = new QLabel();
    lab_nb_versement   = new QLabel();
    et_montant_a_recup = new QLineEdit("O FCFA");
    et_montant_reste   = new QLineEdit("O FCFA");
    et_total_versement = new QLineEdit("0 FCFA");
    et_total_benefice  = new QLineEdit("0 FCFA");
    et_montant_a_recup->setReadOnly(true);
    et_montant_a_recup->setFixedWidth(200);
    et_montant_reste->setReadOnly(true);
    et_montant_reste->setFixedWidth(200);
    et_total_benefice->setReadOnly(true);
    et_total_benefice->setFixedWidth(200);
    et_total_versement->setReadOnly(true);
    et_total_versement->setFixedWidth(200);

    QWidget *tab_totaux = new QWidget();

    QHBoxLayout *lay_infos = new QHBoxLayout();

    QFormLayout *lay_infos1 = new QFormLayout();
    lay_infos1->addRow("Id Prêt : ",lab_pret_selection);
    lay_infos1->addRow("Total paiements : ",et_total_versement);
    lay_infos1->addRow("Bénéfice : ",et_total_benefice);

    QFormLayout *lay_infos2 = new QFormLayout();
    lay_infos2->addRow("Nombre de versement : ",lab_nb_versement);
    lay_infos2->addRow("Montant à recevoir : ",et_montant_a_recup);
    lay_infos2->addRow("Montant restant : ",et_montant_reste);

    lay_infos->addLayout(lay_infos1);
    lay_infos->addLayout(lay_infos2);

    tab_totaux->setLayout(lay_infos);
    widget_tab->addTab(tab_totaux,"Totaux");
    widget_tab->addTab(lab_avaliseur_pret,"Avaliseur");

    return widget_tab;
}

QString FenPrincipale::chargerFichier(const QString nom){
    QFile fichier(nom);
    if(!fichier.open(QIODevice::ReadOnly | QIODevice::Text)) return "";
    QString contenu = fichier.readAll();
    fichier.close();
    return contenu;
}

void FenPrincipale::quitter(){
    int reponse = QMessageBox::question(this,"Quitter","Etes-vous sûr de vouloir quitter ?",QMessageBox::Yes|QMessageBox::No);
    if(reponse==QMessageBox::Yes){
        qApp->closeAllWindows();
        qApp->quit();
    }
}

void FenPrincipale::afficherAcceuil(){
    desactiverSuppression();
    rafraichirInfosAcceuil();
    lay_pager->setCurrentWidget(wid_acceuil);
}

void FenPrincipale::afficherPrets(){
    desactiverSuppression();
    rafraichirInfosPrets(vue_prets->currentIndex());

    QString etatPret = model_prets->data(model_prets->index(vue_prets->currentIndex().row(),1)).toString();

    if(montantRestant!=0 && etatPret=="Soldé"){ //Un versement rendant le prêt soldé a été suppr

        QSqlQuery req;
        //Recup de idPret depuis la vue

        QString idPret = model_prets->data(model_prets->index(vue_prets->currentIndex().row(),0)).toString();
        req.prepare("UPDATE Pret SET etatPret='Actif' "
                             "WHERE idPret=:idPret");
        req.bindValue(":idPret",idPret);
        if(!req.exec()){
            QMessageBox::critical(this,"Erreur","Erreur rencontrée : "+req.lastError().text());
            return;
        }

    }
    model_prets->select();
    vue_prets->setModel(model_prets);
    lay_pager->setCurrentWidget(wid_prets);
}

void FenPrincipale::afficherClients(){
    desactiverSuppression();
    lay_pager->setCurrentWidget(wid_clients);
}

void FenPrincipale::afficherAvaliseurs(){
    desactiverSuppression();
    lay_pager->setCurrentWidget(wid_avaliseurs);
}

void FenPrincipale::afficherVersements(){
    desactiverSuppression();
    lay_pager->setCurrentWidget(wid_versements);
}

void FenPrincipale::demarrerCalculatrice(){
    desactiverSuppression();
    QProcess *processCalc = new QProcess();
    processCalc->start("calc",QStringList());
}

void FenPrincipale::demarrerNotes(){
    desactiverSuppression();
    QProcess *processNotes = new QProcess();
    processNotes->start("notepad",QStringList());
}

void FenPrincipale::rafraichirInfosPrets(QModelIndex index){

    //reinitialiser l'affichage
    lab_avaliseur_pret->setText("");

    int idPret = model_prets->data(model_prets->index(index.row(),0)).toInt();
    lab_pret_selection->setText(QString::number(idPret));

    //Afficher les totaux
    qint64 montantPret = model_prets->data(model_prets->index(index.row(),9)).toFloat();
    long double txInteret  = model_prets->data(model_prets->index(index.row(),10)).toDouble();
    qint64 montantRecu;

    qint64 benefice = (qint64) qCeil(txInteret*montantPret);
    et_total_benefice->setText(QString::number(benefice)+ " FCFA");
    et_montant_a_recup->setText(QString::number(montantPret+benefice) + " FCFA");

    QSqlQuery requete;
    requete.prepare("SELECT SUM(montantVersement) AS TotalPaye, COUNT(*) AS Nbre FROM "+TABLE_VERSEMENT+" WHERE idPret=:idPret");
    requete.bindValue(":idPret",idPret);

    if(!requete.exec()) return;
    if(requete.next()){
        montantRecu = requete.value(0).toLongLong(); // Total montant recu du client
        et_total_versement->setText(QString::number(montantRecu)+" FCFA");
        lab_nb_versement->setText(requete.value(1).toString());
    }

    montantRestant = montantPret+benefice-montantRecu;
    et_montant_reste->setText(QString::number(montantRestant)+ " FCFA");

    //Afficher l'avaliseur
    QSqlQuery requeteAva;
    requeteAva.prepare("SELECT A.nom, A.prenoms, A.adresse, A.telephone, A.profession "
                    "FROM Pret P INNER JOIN Avaliseur A "
                    "ON P.idAvaliseur=A.idAvaliseur "
                    "WHERE idPret=:idPret");
    requeteAva.bindValue(":idPret",idPret);

    if(!requeteAva.exec()) return;
    while(requeteAva.next()){

        lab_avaliseur_pret->setText("  Nom : "+requeteAva.value(0).toString()+" "+requeteAva.value(1).toString()+"\n"
                                    "  Profession : "+requeteAva.value(4).toString()+"\n"
                                    "  Adresse : "+requeteAva.value(2).toString()+"\n"
                                    "  Téléphone : "+requeteAva.value(3).toString());
    }

}

void FenPrincipale::remplirCombosPrets(){

    //Taux d'interet
    QSqlQuery reqTaux;
    if(!reqTaux.exec("SELECT DISTINCT TauxInteret FROM Pret")) return;
    while(reqTaux.next()) cmb_taux_interet->addItem(QString::number(reqTaux.value(0).toDouble()));

    //Montant prêt
    QSqlQuery reqMnt;
    if(!reqMnt.exec("SELECT DISTINCT Montant FROM Pret")) return;
    while(reqMnt.next()) cmb_montant_pret->addItem(reqMnt.value(0).toString());

    //Date Echeance
    QSqlQuery reqEch;
    if(!reqEch.exec("SELECT DISTINCT DateEcheance FROM Pret")) return;
    while(reqEch.next()) cmb_date_echeance->addItem(reqEch.value(0).toString());

    //Client
    QSqlQuery reqCli;
    if(!reqCli.exec("SELECT NomCli, Prenoms FROM Client")) return;
    while(reqCli.next()) cmb_client->addItem(reqCli.value(0).toString()+" "+reqCli.value(1).toString());

    //Avaliseur
    QSqlQuery reqAva;
    if(!reqAva.exec("SELECT idAvaliseur, nom, prenoms FROM Avaliseur")) return;
    while(reqAva.next()) cmb_avaliseur->addItem(reqAva.value(1).toString()+" "+reqAva.value(2).toString(),reqAva.value(0));
}

void FenPrincipale::enreg_modif_avalis(){
    if(model_avaliseurs->submitAll()){
         QMessageBox::information(this,"Avaliseurs","Modifications enregistrées avec succès !");
         ajouterHistorique("Modifications d'informations sur la liste des avaliseurs");
    }else{
        QMessageBox::critical(this,"Avaliseurs","Echec de l'enregistrement : "+model_avaliseurs->lastError().text());
    }
}

void FenPrincipale::enreg_modif_clients(){
    if(model_clients->submitAll()){
         QMessageBox::information(this,"Clients","Modifications enregistrées avec succès !");
         ajouterHistorique("Modifications d'informations sur la liste des clients");
    }else{
        QMessageBox::critical(this,"Clients","Echec de l'enregistrement : "+model_clients->lastError().text());
    }
}

void FenPrincipale::filtrerListePrets(){

    QString etatPret, typeGarantie, modalite, txInteret, montant,
            dateEcheance,nomEntier, nomClient="%", prenCli="%", idAvaliseur;

    //Recuperation des valeurs des combos
    if(cmb_etat_pret->currentIndex()==0) etatPret = cmb_etat_pret->currentData().toString();
    else etatPret = cmb_etat_pret->currentText();

    if(cmb_type_garantie->currentIndex()==0) typeGarantie = cmb_type_garantie->currentData().toString();
    else typeGarantie = cmb_type_garantie->currentText();

    if(cmb_mode_paie->currentIndex()==0) modalite = cmb_mode_paie->currentData().toString();
    else modalite = cmb_mode_paie->currentText();

    if(cmb_taux_interet->currentIndex()==0) txInteret = cmb_taux_interet->currentData().toString();
    else txInteret = cmb_taux_interet->currentText();

    if(cmb_montant_pret->currentIndex()==0) montant = cmb_montant_pret->currentData().toString();
    else montant = cmb_montant_pret->currentText();

    if(cmb_date_echeance->currentIndex()==0) dateEcheance = cmb_date_echeance->currentData().toString();
    else dateEcheance = cmb_date_echeance->currentText();

    //Client
    nomEntier = cmb_client->currentText();
    if(nomEntier!="Tous"){
        nomClient = nomEntier.section(" ",0,0);
        prenCli   = nomEntier.section(" ",1);
    }

    //Avaliseur
    idAvaliseur = cmb_avaliseur->currentData().toString();


    //Construction du filtre
    QString filtre =
            "etatPret LIKE '"+etatPret+"' AND "
            "Modalite LIKE '"+modalite+"' AND "
            "TauxInteret LIKE '"+txInteret+"' AND "
            "Montant LIKE '"+montant+"' AND "
            "DateEcheance LIKE '"+dateEcheance+"' AND ";
    if(typeGarantie!="%") filtre += "type LIKE '"+typeGarantie+"' AND "; //Erreur résolue :si type(filtre)=% et type=''
            filtre+=
            "NomCli LIKE '"+nomClient+"' AND "
            "Prenoms LIKE '"+prenCli+"'";
    if(idAvaliseur!="%")  filtre += " AND idAvaliseur='"+idAvaliseur+"'"; //Même erreur

    model_prets->setFilter(filtre);
    model_prets->setJoinMode(QSqlRelationalTableModel::LeftJoin);
    model_prets->select();
    vue_prets->setModel(model_prets);
    if(model_prets->rowCount()==0) QMessageBox::information(this,"Liste des prêts","Aucune correspondance trouvée");
    else QMessageBox::information(this,"Liste des prêts",QString::number(model_prets->rowCount())+" correspondance(s) trouvée(s)");

}

void FenPrincipale::remplirCombosVers(){
    //Client
    QSqlQuery reqCli;
    if(!reqCli.exec("SELECT NomCli, Prenoms FROM Client")) return;
    while(reqCli.next()) cmb_client_vers->addItem(reqCli.value(0).toString()+" "+reqCli.value(1).toString());

    //Date de versement
    QSqlQuery reqDateVers;
    if(!reqDateVers.exec("SELECT DISTINCT dateVersement FROM Versement")) return;
    while(reqDateVers.next()) cmb_date_vers->addItem(reqDateVers.value(0).toString());
}

void FenPrincipale::filtrerListeVersements(){
    QString nomEntier, nomCli="%", prenCli="%", dateVersement="%", filtre;

    if(cmb_client_vers->currentIndex()!=0)
    {
        nomEntier = cmb_client_vers->currentText();
        nomCli = nomEntier.section(" ",0,0);
        prenCli= nomEntier.section(" ",1);
    }

    if(cmb_date_vers->currentIndex()!=0) dateVersement = cmb_date_vers->currentText();

    filtre = "NomCli LIKE '"+nomCli+"' AND "
             "Prenoms LIKE '"+prenCli+"' AND "
             "dateVersement LIKE '"+dateVersement+"'";
    model_versements->setFilter(filtre);
    model_versements->select();
    vue_versements->setModel(model_versements);
    if(model_versements->rowCount()==0) QMessageBox::information(this,"Liste des versements","Aucune correspondance trouvée");
    else QMessageBox::information(this,"Liste des versements",QString::number(model_versements->rowCount())+" correspondance(s) trouvée(s)");
}

void FenPrincipale::activerSupClient(){
    if(!actSupClient->isEnabled()) actSupClient->setEnabled(true);
}

void FenPrincipale::activerSupAvaliseur(){
    if(!actSupAvaliseur->isEnabled()) actSupAvaliseur->setEnabled(true);
}

void FenPrincipale::activerSupPret(){
    if(!actSupPret->isEnabled()) actSupPret->setEnabled(true);
    if(!actNouveauVersement->isEnabled()) actNouveauVersement->setEnabled(true);
}

void FenPrincipale::activerSupVersement(){
    if(!actSupVersement->isEnabled()) actSupVersement->setEnabled(true);
}

void FenPrincipale::desactiverSuppression(){
    actSupAvaliseur->setEnabled(false);
    actSupClient->setEnabled(false);
    actSupPret->setEnabled(false);
    actSupVersement->setEnabled(false);
    actNouveauVersement->setEnabled(false);
}

void FenPrincipale::supprClient(){
    int ligne = vue_clients->currentIndex().row();

    QString nom, prenoms;
    QModelIndex index = model_clients->index(ligne,1); //index nom
    nom = model_clients->data(index).toString();
    index = model_clients->index(ligne,2); //index prénoms
    prenoms = model_clients->data(index).toString();

    int reponse = QMessageBox::question(this,"Supprimer Client","Etes-vous sûr de vouloir supprimer le client "+nom+" "
                                    ""+prenoms+" ainsi que les informations"
                                    " sur les prêts, versements et garanties le concernant ?"
                                    , QMessageBox::Yes|QMessageBox::Cancel);

    if(reponse!=QMessageBox::Yes) return;

    index =model_clients->index(ligne,0); //index id
    QString idClient = model_clients->data(index).toString();

    //Recupérer les IDs de tous ses prêts
    QSqlQuery reqSupPretsClient;
    if(!reqSupPretsClient.exec("SELECT idPret FROM Pret "
                               "WHERE idClient="+idClient+"")){
        QMessageBox::critical(this,"Erreur","Suppression impossible : "+reqSupPretsClient.lastError().text());
        return;
    }
    //Supprimer ts ses prêts, versements et garantie
    while(reqSupPretsClient.next()){
        supprPret(reqSupPretsClient.value(0).toString());
    }

    if(model_clients->removeRow(ligne)){
        model_clients->submitAll();
        model_clients->select();
        vue_clients->setModel(model_clients);
        ajouterHistorique("Suppression du client "+nom+" "+prenoms);
    }
}

void FenPrincipale::supprAvaliseur(){
    int ligne = vue_avaliseurs->currentIndex().row();

    QString nom, prenoms;
    QModelIndex index = model_avaliseurs->index(ligne,1); //index nom
    nom = model_avaliseurs->data(index).toString();
    index = model_avaliseurs->index(ligne,2); //index prénoms
    prenoms = model_avaliseurs->data(index).toString();

    int reponse = QMessageBox::question(this,"Supprimer Avaliseur","Etes-vous sûr de vouloir supprimer l'avaliseur "+nom+" "
                         ""+prenoms+" ?", QMessageBox::Yes|QMessageBox::Cancel);

    if(reponse!=QMessageBox::Yes) return;

    //Mettre NULL a tous les prêts qui ont cet avaliseur
    index = model_avaliseurs->index(ligne,0); //index idAvaliseur
    QString idAvaliseur = model_avaliseurs->data(index).toString();
    QSqlQuery req;
    if(!req.exec("UPDATE Pret SET idAvaliseur=NULL "
                 "WHERE idAvaliseur="+idAvaliseur+""))
    {
        QMessageBox::critical(this,"Erreur","Suppression impossible : "+req.lastError().text());
        return;
    }

    model_prets->select();
    vue_prets->setModel(model_prets); //reactualiser la liste des prêts

    //Supprimer maintenant l'avaliseur
    if(!model_avaliseurs->removeRow(ligne)){
        QMessageBox::critical(this,"Erreur","Suppression impossible : "+model_avaliseurs->lastError().text());
    }else{
        model_avaliseurs->submitAll();
        model_avaliseurs->select();
        vue_avaliseurs->setModel(model_avaliseurs);
        ajouterHistorique("Suppression de l'avaliseur' "+nom+" "+prenoms);
    }
}

void FenPrincipale::supprPret(QString idPret){

    if(idPret=="") //Si l'id du prêt n'est  pas explicitiment passé en paramètre
    {
        //Recupérer l'id du prêt
        int ligne = vue_prets->currentIndex().row();
        QModelIndex index_id  = model_prets->index(ligne,0);
        idPret = model_prets->data(index_id).toString();

        int reponse = QMessageBox::question(this,"Supprimer prêt","Etes-vous sûr de vouloir supprimer"
                                                 " le prêt d' ID : "+idPret+" ainsi que les informations"
                                                 " sur les versements et la garantie le concernant ?",
                                                 QMessageBox::Yes|QMessageBox::Cancel);

        if(reponse != QMessageBox::Yes) return ;
    }

    QSqlQuery reqSupPret;
    if(!reqSupPret.exec("DELETE FROM Versement WHERE idPret="+idPret+"")){  //Supprimer ses versements
        ajouterHistorique("Suppression des versements du prêt d' ID : "+idPret);
        QMessageBox::critical(this,"Erreur","Suppression impossible : "+reqSupPret.lastError().text());
    }

    if(!reqSupPret.exec("DELETE FROM Garantie WHERE "
                    "idGarantie=(SELECT idGarantie FROM Pret WHERE idPret="+idPret+")")){ //Supprimer la garantie
        QMessageBox::critical(this,"Erreur","Suppression impossible : "+reqSupPret.lastError().text());
    }

    if(!reqSupPret.exec("DELETE FROM Pret WHERE idPret="+idPret+"")){
        QMessageBox::critical(this,"Erreur","Suppression impossible : "+reqSupPret.lastError().text());
    }else{

        model_prets->select();
        vue_prets->setModel(model_prets);
        model_versements->select();
        vue_versements->setModel(model_versements);
        ajouterHistorique("Suppression du prêt d' ID : "+idPret);
    }
}

void FenPrincipale::supprVersement(){
    //Recupérer l'id du versement
    int ligne = vue_versements->currentIndex().row();
    QModelIndex index_id  = model_versements->index(ligne,0);
    QString idVers = model_versements->data(index_id).toString();

    int reponse = QMessageBox::question(this,"Supprimer versement","Etes-vous sûr de vouloir supprimer"
                                                " le versement d' ID : "+idVers+" ?",QMessageBox::Yes|QMessageBox::Cancel);
    if(reponse != QMessageBox::Yes) return ;

    QSqlQuery reqSupVers;
    if(!reqSupVers.exec("DELETE FROM Versement WHERE idVersement="+idVers+"")){
        QMessageBox::critical(this,"Erreur","Suppression impossible : "+reqSupVers.lastError().text());
    }else{
        model_versements->select();
        vue_versements->setModel(model_versements);
        ajouterHistorique("Suppression du versement d' ID : "+idVers);
    }
}

void FenPrincipale::rafraichirInfosAcceuil(){

    list_alertes->clear(); //Vider la liste
    list_historique->clear();
    actualiserHistorique();
    lab_nb_alerte->setText("<h2> Alertes échéances (0)</h2>");
    lab_benef_actuel->setText("0 FCFA");
    lab_benef_total->setText("0 FCFA");

    QSqlQuery reqBenef;
    if(!reqBenef.exec("SELECT SUM(Montant*TauxInteret) AS BenefActif "
                      "FROM Pret "
                      "WHERE etatPret='Soldé'"))
        return;
    while(reqBenef.next()) lab_benef_actuel->setText(QString::number(qCeil(reqBenef.value(0).toLongLong()))+" FCFA");

    if(!reqBenef.exec("SELECT SUM(Montant*TauxInteret) AS BenefTotal "
                      "FROM Pret"))
        return;
    while(reqBenef.next()) lab_benef_total->setText(QString::number(qCeil(reqBenef.value(0).toLongLong()))+" FCFA");

    QSqlQuery reqEcheProch;

    QDate today = QDate::currentDate(),
          AweekAfterToday = today.addDays(7),
          dateEcheance;

    if(!reqEcheProch.exec("SELECT NomCli, Prenoms, DateEcheance, "
                          "Montant*(1+TauxInteret) AS MontantDu, "
                          "((Montant*(1+TauxInteret))-SUM(montantVersement)) AS MontantRestant "
                          "FROM Client C INNER JOIN Pret P "
                          "ON C.idClient=P.idClient "
                          "LEFT JOIN Versement V "
                          "ON P.idPret=V.idPret "
                          "WHERE etatPret='Actif' "
                          "GROUP BY P.idPret "
                          "ORDER BY DateEcheance ASC"
                          )) return;

    QString item, date;
    QStringList listAjdhui, listFuture, listPasseNonPaye;

    while(reqEcheProch.next()){

        date = reqEcheProch.value("DateEcheance").toString();
        dateEcheance = QDate::fromString(date,"dd'/'MM'/'yyyy");

        item = reqEcheProch.value("NomCli").toString()+" "
               ""+reqEcheProch.value("Prenoms").toString()+"\t\t"
               ""+reqEcheProch.value("DateEcheance").toString();

        if(reqEcheProch.value("MontantRestant").toString()==""){  //Aucun versement
            item += "  "+QString::number(reqEcheProch.value("MontantDu").toLongLong())+" FCFA";
        }else{                                                    //Versements effectués
            item += "  "+QString::number(reqEcheProch.value("MontantRestant").toLongLong())+" FCFA";
        }


        if(today<dateEcheance && dateEcheance<=AweekAfterToday){ //Echeance dans les 7 jours
            listFuture << item;
        }

        if(today==dateEcheance){ //Echeance aujourdhui
            listAjdhui << item;
        }

        if(dateEcheance<today){ //Echeance passée mais prêt non soldé
            listPasseNonPaye << item;
        }
    }

    //Ajout des alertes
    list_alertes->addItem("\t\tECHÉANCES AUJOURD'HUI");
    list_alertes->addItems(listAjdhui);

    list_alertes->addItem("");
    list_alertes->addItem("\t\tECHÉANCES PASSÉES !");
    list_alertes->addItems(listPasseNonPaye);

    list_alertes->addItem("");
    list_alertes->addItem("\t\tECHÉANCES SEPT(7) PROCHAINS JOURS");
    list_alertes->addItems(listFuture);

    //Ajout du nombre tota d'alertes
    lab_nb_alerte->setText("<h2>Alertes échéances ("+QString::number(list_alertes->count()-5)+")</h2>");
}

void FenPrincipale::afficherApropos(){
    QDialog *dialog_Apropos = new QDialog(this);
    QVBoxLayout *lay_dial_Apropos = new QVBoxLayout();
    QHBoxLayout *en_tete = new QHBoxLayout();
    QHBoxLayout *bas_page = new QHBoxLayout();


    QLabel *image_app = new QLabel();
    image_app->setPixmap(QPixmap("images/ic_app.png"));
    image_app->setFixedSize(100,100);
    image_app->setScaledContents(true);
    image_app->setAlignment(Qt::AlignVCenter);

    QPushButton *btn_ok = new QPushButton("Ok");
    btn_ok->setFixedHeight(30);
    connect(btn_ok,SIGNAL(clicked(bool)),dialog_Apropos,SLOT(close()));

    QString text =
                   "<h2>Gestionnaire de prêts</h2>"
                   "<h3>version 1.0.0</h3>"
                   "Loan Manager est un gestionnaire de prêts "
                   "permet la gestion de la liste des clients,"
                   " des avaliseurs, des garanties d'emprunts. "

                   "La gestion des listes de prêts est un module "
                   " pilier de l'application et offre la possibilité "
                   " de rechercher aisément des prêts bien précis."
                   "Un bilan vous est dressé par rapport au prêt permettant ainsi d'avoir"
                   " des informations rapidement et précises sur eux. "

                   "Ne manquez plus aucune date d'écheance grâce"
                   " au module vous affichant les prêts arrivant à échéance dans la "
                   "semaine suivante (7 jours au plus), dès l'ouverture de l'application.";

    QLabel *textApropos = new QLabel(text);
    textApropos->setTextFormat(Qt::RichText);
    textApropos->setWordWrap(true);
    textApropos->setStyleSheet("background-color:white");
    textApropos->setAlignment(Qt::AlignHCenter);

    en_tete->addSpacing(200);
    en_tete->addWidget(image_app);
    en_tete->addSpacing(200);

    bas_page->addSpacing(350);
    bas_page->addWidget(btn_ok);

    lay_dial_Apropos->addLayout(en_tete);
    lay_dial_Apropos->addSpacing(10);
    lay_dial_Apropos->addWidget(textApropos);
    lay_dial_Apropos->addLayout(bas_page);

    dialog_Apropos->setLayout(lay_dial_Apropos);
    dialog_Apropos->setFixedSize(500,470);
    dialog_Apropos->exec();
}

void FenPrincipale::ajouterHistorique(QString description){
    QFile fichierHist(FILE_HISTORIQUE);
    if(!fichierHist.open(QIODevice::Append | QIODevice::Text))
        return;

    QString hist = "\n"+QDate::currentDate().toString("dd/MM/yyyy")+ " "
                   "" +QTime::currentTime().toString("hh:mm:ss")+ " "+description;

    QTextStream out(&fichierHist);
    out << hist;
}

void FenPrincipale::actualiserHistorique(){
    list_historique->clear();
    QFile fichierHist(FILE_HISTORIQUE);
    if(!fichierHist.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QString itemHist;
    QTextStream in(&fichierHist);
    while(!in.atEnd()){
        itemHist = in.readLine();
        list_historique->addItem(itemHist);
    }
}

void FenPrincipale::viderHistorique(){
    int reponse = QMessageBox::question(this,"Historique",
                                        "Etes-vous sûr de vouloir vider toute l'historique des opérations ?",
                                        QMessageBox::Yes|QMessageBox::Cancel);
    if(reponse!=QMessageBox::Yes) return ;

    bool ok = false;
    QString pass =
            QInputDialog::getText(this,"Authentification",
                                             "Identifiant : <b>"+USER_NAME+"</b> | "
                                             "Entrez votre mot de passe :",QLineEdit::Password,
                                             QString(),&ok);
    if(ok){
        if(pass==PASS){
            QFile fichierHist(FILE_HISTORIQUE);
            if(!fichierHist.open(QIODevice::WriteOnly | QIODevice::Text))
                return;
            QTextStream out(&fichierHist);
            out << "";
            actualiserHistorique();
        }else{
            QMessageBox::critical(this,"Historique","Echec authentification");
        }
    }
}

void FenPrincipale::nouveauVersement(){
    dialog_nv_versement = new QDialog(this);

    QVBoxLayout *lay_dialog_nv_vers = new QVBoxLayout();
    QLabel *lab_infos = new QLabel();

    spb_montantVersement = new QDoubleSpinBox();
    //Le montant du versement ne dépasse pas le montant restant à payer
    spb_montantVersement->setRange(0,montantRestant);
    spb_montantVersement->setValue(0);
    spb_montantVersement->setSuffix(" FCFA");
    spb_montantVersement->setSingleStep(1000);

    QPushButton *btn_add_versement = new QPushButton("Ajouter versement");

    QString idPret = lab_pret_selection->text();
    QString idClient, nomClient, prenClient;

    QSqlQuery req;
    if(!req.exec("SELECT C.idClient, NomCli, Prenoms "
                 "FROM Pret P INNER JOIN Client C "
                 "ON P.idClient=C.idClient "
                 "WHERE idPret="+idPret+"")){
        QMessageBox::critical(this,"Erreur","Echec de l'opération : "+req.lastError().text());
        return;
    }


    //On récupére le résultat
    if(req.next()){
        idClient = req.value("idClient").toString();
        nomClient = req.value("NomCli").toString();
        prenClient = req.value("Prenoms").toString();

        //Initialisation pr l'enregistrement éventuel du versement
        VERSE_ID_CLIENT = idClient;
        VERSE_ID_PRET = idPret;
    }else{
        QMessageBox::critical(this,"Erreur","Echec de l'opération : "+req.lastError().text());
        return;
    }

    lab_infos->setText("ID Prêt : "+ idPret+"\n"
                       "Client : "+nomClient+" "+prenClient+"\n"
                       "Montant restant : "+ et_montant_reste->text()+"\n\n"
                       "Montant versement");

    QHBoxLayout *lay_bas = new QHBoxLayout();
    lay_bas->addSpacing(100);
    lay_bas->addWidget(btn_add_versement);

    lay_dialog_nv_vers->addWidget(lab_infos);
    lay_dialog_nv_vers->addWidget(spb_montantVersement);
    lay_dialog_nv_vers->addLayout(lay_bas);

    //Connexion
    connect(btn_add_versement,SIGNAL(clicked(bool)),this,SLOT(enregVersement()));

    //Affichage du dialogue
    dialog_nv_versement->setLayout(lay_dialog_nv_vers);
    dialog_nv_versement->setWindowTitle("Nouveau versement");
    dialog_nv_versement->setFixedSize(300,200);
    dialog_nv_versement->show();

}

void FenPrincipale::enregVersement(){

    QString idClient = VERSE_ID_CLIENT, idPret = VERSE_ID_PRET;

    qlonglong mntntVersement = (qlonglong) spb_montantVersement->value();

    if(mntntVersement==0){
        QMessageBox::warning(this,"Nouveau versement","Entrez un montant non nul");
        return;
    }


    QSqlQuery req_nv_vers ;
    req_nv_vers.prepare("INSERT INTO Versement(idClient,idPret,montantVersement,dateVersement) "
                        "VALUES(:idClient,:idPret,:montantVersement,:dateVersement)");
    req_nv_vers.bindValue(":idClient",idClient);
    req_nv_vers.bindValue(":idPret",idPret);
    req_nv_vers.bindValue(":montantVersement",mntntVersement);
    req_nv_vers.bindValue(":dateVersement",QDate::currentDate().toString("dd/MM/yyyy"));

    if(!req_nv_vers.exec()){
        QMessageBox::critical(this,"Erreur","Ajout non effectué : "+req_nv_vers.lastError().text());
    }else{
        model_versements->select();
        vue_versements->setModel(model_versements);
        dialog_nv_versement->close();
        rafraichirInfosPrets(vue_prets->currentIndex());
        if(mntntVersement==spb_montantVersement->maximum()){ // Sil a tout payé mettre le prêt à soldé
            req_nv_vers.prepare("UPDATE Pret SET etatPret='Soldé' "
                                 "WHERE idPret=:idPret");
            req_nv_vers.bindValue(":idPret",idPret);
            if(!req_nv_vers.exec()){
                QMessageBox::critical(this,"Erreur","Erreur rencontrée : "+req_nv_vers.lastError().text());
                return;
            }
            model_prets->select();
            vue_prets->setModel(model_prets);
        }
        QMessageBox::information(this,"Nouveau versement","Opération réussie !");
    }
}

void FenPrincipale::lancerLeTimer(){

    if(qApp->applicationState()==Qt::ApplicationActive){
        timerAuthentification->stop();
    }else{
        if(login_ok()){
            timerAuthentification->start();
        }
    }
}

bool FenPrincipale::utilisateurExiste(){
    QSqlQuery requete;
    if(!requete.exec("SELECT idUser FROM Utilisateur")){
        QMessageBox::critical(this,"Erreur critique","Echec de l'accès aux données : "+requete.lastError().text());
        quitterSiNonAuthentifier();
    }else{
        if(requete.next()) return true;
    }
    return false;
}

/*#####################################################################################################*/

//****************************************   PATCH Jr   *************************************************

void FenPrincipale::afficherDialogueNouveauClient()
{
      dialog_nouveau_client = new QDialog(this);
      dialog_nouveau_client->setLayout(ui_enreg_client_form());
      dialog_nouveau_client->setFixedSize(420,320);
      dialog_nouveau_client->setStyleSheet(chargerFichier(STYLE_LOGIN_FORM));
      dialog_nouveau_client->setWindowTitle("Client");
      dialog_nouveau_client->show();
}

QVBoxLayout* FenPrincipale::ui_enreg_client_form(){

    QVBoxLayout *lay_dialog = new QVBoxLayout();

      et_client_nom = new QLineEdit;
      et_client_prenom = new QLineEdit;
      et_client_telephone = new QLineEdit;
      et_client_adresse = new QLineEdit;
      et_client_profession = new QLineEdit;
      et_client_societe = new QLineEdit;
      date_nais_client = new QDateEdit;
      date_nais_client->setDate((QDate::currentDate()));

      QPushButton *btn_enregistrer = new QPushButton("Enregistrer");
      QPushButton *btn_quitter = new QPushButton("Fermer");
      QFormLayout *formLayout = new QFormLayout;
      QHBoxLayout *lay_btn = new QHBoxLayout;
      QLabel *lab_en_tete = new QLabel("<h3>Nouveau client</h3>");
      lab_en_tete->setFixedHeight(50);

      formLayout->addRow("Nom : ",et_client_nom);
      formLayout->addRow("Prénom : ",et_client_prenom);
      formLayout->addRow("Date de naissance:",date_nais_client);
      formLayout->addRow("telephone: ",et_client_telephone);
      formLayout->addRow("Adresse : ",et_client_adresse);
      formLayout->addRow("Profession: ",et_client_profession);
      formLayout->addRow("Société: ",et_client_societe);

      lay_btn->addSpacing(150);
      lay_btn->addWidget(btn_enregistrer);
      lay_btn->addWidget(btn_quitter);

      lay_dialog->addWidget(lab_en_tete);
      lay_dialog->addLayout(formLayout);
      lay_dialog->addLayout(lay_btn);

      connect(btn_enregistrer,SIGNAL(clicked(bool)),this,SLOT(enreg_client()));
      connect(btn_quitter,SIGNAL(clicked(bool)),this,SLOT(quitter_client()));

      return lay_dialog;
}

void FenPrincipale::quitter_client()
{
    dialog_nouveau_client->close();
}

void FenPrincipale::enreg_client()
{
  if(et_client_nom->text().isEmpty()||et_client_prenom->text().isEmpty()||et_client_telephone->text().isEmpty()||et_client_adresse->text().isEmpty()||et_client_profession->text().isEmpty()||et_client_societe->text().isEmpty())
  {
       QMessageBox::warning(this,"Enregistrement","Veuillez renseigner tous les champs");
  }
  else
  {
      QSqlQuery req;
      req.prepare("INSERT INTO Client(NomCli,Prenoms,dateNaissance,telephone,Adresse,Profession,societe) "
                  "VALUES(:NomCli,:Prenoms,:dateNaissance,:telephone,:Adresse,:Profession,:societe)");
      req.bindValue(":NomCli",et_client_nom->text());
      req.bindValue(":Prenoms",et_client_prenom->text());
      req.bindValue(":dateNaissance",date_nais_client->text());
      req.bindValue(":telephone",et_client_telephone->text());
      req.bindValue(":Adresse",et_client_adresse->text());
      req.bindValue(":Profession",et_client_profession->text());
      req.bindValue(":societe",et_client_societe->text());
      req.exec();
      QMessageBox::information(this,"Information","Enrégistrement réussi");
      et_client_nom->setText(" ");
      et_client_prenom->setText(" ");
      date_nais_client->setDate(QDate::currentDate());
      et_client_telephone->setText(" ");
      et_client_adresse->setText(" ");
      et_client_profession->setText(" ");
      et_client_societe->setText(" ");
  }
}

void FenPrincipale::afficherDialogueNouveauPret()
{
      dialog_nouveau_pret = new QDialog(this);
      dialog_nouveau_pret->setLayout(ui_enreg_pret_form());
      dialog_nouveau_pret->setFixedSize(500,430);
      dialog_nouveau_pret->setStyleSheet(chargerFichier(STYLE_LOGIN_FORM));
      dialog_nouveau_pret->setWindowTitle("Nouveau prêt");
      dialog_nouveau_pret->show();
}

QVBoxLayout* FenPrincipale::ui_enreg_pret_form()
{
      cmb_enregpret_client     = new QComboBox();
      cmb_enregpret_avaliseur = new QComboBox();
      cmb_enregpret_avaliseur->addItem("Aucun","NULL");
      cmb_enregpret_garantie = new QComboBox();

      cmb_enregpret_garantie->addItem("Aucune","NULL");
      cmb_taux_interet_pret = new QComboBox();

      cmb_taux_interet_pret->addItem("0%",0);
      cmb_taux_interet_pret->addItem("5%",0.05);
      cmb_taux_interet_pret->addItem("10%",0.1);
      cmb_taux_interet_pret->addItem("15%",0.15);
      cmb_taux_interet_pret->addItem("20%",0.2);
      cmb_taux_interet_pret->addItem("25%",0.25);
      cmb_taux_interet_pret->addItem("30%",0.3);
      cmb_mode_paiement     = new QComboBox();
      cmb_mode_paiement->addItem("Mensuel");
      cmb_mode_paiement->addItem("Trimestriel");
      cmb_mode_paiement->addItem("Semestriel");
      cmb_mode_paiement->addItem("Annuel");
      cmb_mode_paiement->addItem("Cash");
      et_montant_pret = new QDoubleSpinBox;
      et_montant_pret->setRange(0,999999999999999999);
      //Maximum :999millions 999mille 999 de milliards
      et_solde = new QLineEdit;
      date_echeance_pret = new QDateEdit;
      date_pret= new QDateEdit;
      date_pret->setDate(QDate::currentDate());
      date_echeance_pret->setDate((QDate::currentDate()));


      QPushButton *btn_enregistrer = new QPushButton("Enregistrer");
      QPushButton *btn_quitter = new QPushButton("Fermer");
      QPushButton *btn_Nouvelle_Garantie=new QPushButton("Ajouter garantie");
      btn_Nouvelle_Garantie->setProperty("btnLien",true);
      btn_Nouvelle_Garantie->setFixedWidth(150);
      QLabel *lab_en_tete = new QLabel("<h3>Nouveau prêt</h3>");
      lab_en_tete->setFixedHeight(50);



      QVBoxLayout *lay_dialog = new QVBoxLayout();
      QHBoxLayout *lay_btn = new QHBoxLayout;

      QFormLayout *formLayout = new QFormLayout;
      formLayout->addRow("Client :",cmb_enregpret_client);
      formLayout->addRow("",new QLabel());
      formLayout->addRow("Avaliseur : ",cmb_enregpret_avaliseur);
      formLayout->addRow("",new QLabel());
      formLayout->addRow("Garantie : ",cmb_enregpret_garantie);
      formLayout->addRow("",btn_Nouvelle_Garantie);
      formLayout->addRow("Date du prêt: ",date_pret);
      formLayout->addRow("Montant du prêt: ",et_montant_pret);
      formLayout->addRow("Taux d'intérêt:",cmb_taux_interet_pret);
      formLayout->addRow("Date de l'échéance: ",date_echeance_pret);
      formLayout->addRow("Modalité de paiement : ",cmb_mode_paiement);

      lay_btn->addSpacing(250);
      lay_btn->addWidget(btn_enregistrer);
      lay_btn->addWidget(btn_quitter);

      lay_dialog->addWidget(lab_en_tete);
      lay_dialog->addLayout(formLayout);
      lay_dialog->addLayout(lay_btn);

      //Connexions
      connect(btn_enregistrer,SIGNAL(clicked(bool)),this,SLOT(enreg_pret()));
      connect(btn_quitter,SIGNAL(clicked(bool)),this,SLOT(quitter_pret()));
      connect(btn_Nouvelle_Garantie,SIGNAL(clicked(bool)),this,SLOT(afficherDialogueNouvelleGarantie()));

     //requete de remplissage des QComboBox
          //identité du client
      QSqlQuery reqidcli;
      reqidcli.exec("SELECT idClient,NomCli,Prenoms FROM Client");
      while(reqidcli.next())
      {
          QString idClient=reqidcli.value("idClient").toString();
          QString nomcli=reqidcli.value("NomCli").toString();
          QString prenomcli=reqidcli.value("Prenoms").toString();
          cmb_enregpret_client->addItem(nomcli+" "+prenomcli,idClient);
      }
          //Avaliseur
      QSqlQuery reqidavaliseur;
      reqidavaliseur.exec("SELECT idAvaliseur,nom,prenoms FROM Avaliseur");
      while(reqidavaliseur.next())
      {
          QString idAvaliseur=reqidavaliseur.value("idAvaliseur").toString();
          QString nom=reqidavaliseur.value("nom").toString();
          QString prenom=reqidavaliseur.value("prenoms").toString();
          cmb_enregpret_avaliseur->addItem(nom+" "+prenom,idAvaliseur);
      }
          //Garantie
      QSqlQuery reqidgarantie;
      reqidgarantie.exec("SELECT idGarantie,caracteristiques FROM Garantie");
      while(reqidgarantie.next())
      {
          QString idGarantie=reqidgarantie.value("idGarantie").toString();
          QString caracteristique=reqidgarantie.value("caracteristiques").toString();
          cmb_enregpret_garantie->addItem(caracteristique,idGarantie);
      }

      return lay_dialog;
}

void FenPrincipale::quitter_pret()
{
    dialog_nouveau_pret->close();
}

void FenPrincipale::enreg_pret()
{
  if(et_montant_pret->value()==0)
  {
      QMessageBox::warning(this,"Enregistrement","Veuillez renseigner tous les champs");
      return;
  }
  else
  {
      int reponse  = QMessageBox::question(this,"Prêt","Confirmez-vous l'ajout du nouveau prêt ?",
                                           QMessageBox::Yes|QMessageBox::No);

      if(reponse!=QMessageBox::Yes) return;

          QSqlQuery req;
          req.prepare("INSERT INTO Pret(etatPret,idClient,idGarantie,idAvaliseur,datePret,Montant,TauxInteret,DateEcheance,Modalite)"
                   " VALUES(:etatPret,:idClient,:idGarantie,:idAvaliseur,:datePret,:Montant,:TauxInteret,:DateEcheance,:Modalite)");
          req.bindValue(":etatPret","Actif");
          req.bindValue(":idClient",cmb_enregpret_client->currentData().toString());
          req.bindValue(":idGarantie",cmb_enregpret_garantie->currentData().toString());
          req.bindValue(":idAvaliseur",cmb_enregpret_avaliseur->currentData().toString());
          req.bindValue(":datePret",date_pret->text());
          req.bindValue(":Montant",et_montant_pret->value());
          req.bindValue(":TauxInteret",cmb_taux_interet_pret->currentData().toDouble());
          req.bindValue(":DateEcheance",date_echeance_pret->text());
          req.bindValue(":Modalite",cmb_mode_paiement->currentText());
          req.exec();
          QMessageBox::information(this,"Information","Enrégistrement réussi");
          date_pret->setDate(QDate::currentDate());
          et_montant_pret->setValue(0.0);
          date_echeance_pret->setDate(QDate::currentDate());
  }
}

void FenPrincipale::afficherDialogueNouvelAvaliseur()
{
  dialog_nouvel_avaliseur = new QDialog(this);
  dialog_nouvel_avaliseur->setLayout(ui_enreg_avaliseur_form());
  dialog_nouvel_avaliseur->setFixedSize(400,300);
  dialog_nouvel_avaliseur->setStyleSheet(chargerFichier(STYLE_LOGIN_FORM));
  dialog_nouvel_avaliseur->setWindowTitle("Avaliseur");
  dialog_nouvel_avaliseur->show();
}

QVBoxLayout* FenPrincipale::ui_enreg_avaliseur_form()
{
  et_avaliseur_nom = new QLineEdit;
  et_avaliseur_prenom = new QLineEdit;
  et_avaliseur_profession = new QLineEdit;
  et_avaliseur_societe = new QLineEdit;
  et_avaliseur_telephone = new QLineEdit;
  et_avaliseur_adresse = new QLineEdit;

  QPushButton *btn_enregistrer = new QPushButton("Enregistrer");
  QPushButton *btn_quitter = new QPushButton("Fermer");
  QHBoxLayout *lay_btn = new QHBoxLayout;
  QVBoxLayout *lay_dialog = new QVBoxLayout;
  QLabel *lab_en_tete = new QLabel("<h3>Nouvel avaliseur</h3>");
  lab_en_tete->setFixedHeight(50);

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow("Nom:",et_avaliseur_nom);
  formLayout->addRow("Prénoms: ",et_avaliseur_prenom);
  formLayout->addRow("Profession: ",et_avaliseur_profession);
  formLayout->addRow("Société: ",et_avaliseur_societe);
  formLayout->addRow("Téléphone:",et_avaliseur_telephone);
  formLayout->addRow("Adresse: ",et_avaliseur_adresse);

  lay_btn->addSpacing(120);
  lay_btn->addWidget(btn_enregistrer);
  lay_btn->addWidget(btn_quitter);

  lay_dialog->addWidget(lab_en_tete);
  lay_dialog->addLayout(formLayout);
  lay_dialog->addLayout(lay_btn);

  //Connexions
  connect(btn_enregistrer,SIGNAL(clicked(bool)),this,SLOT(enreg_avaliseur()));
  connect(btn_quitter,SIGNAL(clicked(bool)),this,SLOT(quitter_avaliseur()));
  return lay_dialog;
}

void FenPrincipale::quitter_avaliseur()
{
 dialog_nouvel_avaliseur->close();
}

void FenPrincipale::enreg_avaliseur()
{
  if(et_avaliseur_nom->text().isEmpty()||et_avaliseur_prenom->text().isEmpty()||et_avaliseur_profession->text().isEmpty()||et_avaliseur_societe->text().isEmpty()||et_avaliseur_telephone->text().isEmpty()||et_avaliseur_adresse->text().isEmpty())
  {
      QMessageBox::warning(this,"Enregistrement","Veuillez renseigner tous les champs");
      return;
  }else
  {
      QSqlQuery req;
      req.prepare("INSERT INTO Avaliseur(nom,prenoms,profession,societe,telephone,adresse) "
               "VALUES (:nom,:prenoms,:profession,:societe,:telephone,:adresse)");
      req.bindValue(":nom",et_avaliseur_nom->text());
      req.bindValue(":prenoms",et_avaliseur_prenom->text());
      req.bindValue(":profession",et_avaliseur_profession->text());
      req.bindValue(":societe",et_avaliseur_societe->text());
      req.bindValue(":telephone",et_avaliseur_telephone->text());
      req.bindValue(":adresse",et_avaliseur_adresse->text());
      req.exec();
      QMessageBox::information(this,"Information","Enrégistrement réussi");
      et_avaliseur_nom->setText(" ");
      et_avaliseur_prenom->setText(" ");
      et_avaliseur_profession->setText(" ");
      et_avaliseur_societe->setText(" ");
      et_avaliseur_telephone->setText(" ");
      et_avaliseur_adresse->setText(" ");
  }
}

void FenPrincipale::afficherDialogueNouvelleGarantie()
{
      dialog_nouvelle_garantie = new QDialog(this);
      dialog_nouvelle_garantie->setLayout(ui_enreg_garantie_form());
      dialog_nouvelle_garantie->setFixedSize(420,230);
      dialog_nouvelle_garantie->setStyleSheet(chargerFichier(STYLE_LOGIN_FORM));
      dialog_nouvelle_garantie->setWindowTitle("Nouvelle garantie");
      dialog_nouvelle_garantie->show();
}

QVBoxLayout* FenPrincipale::ui_enreg_garantie_form()
{
      cmb_add_type_garantie = new QComboBox();
      cmb_add_type_garantie->addItem("Gage");
      cmb_add_type_garantie->addItem("Nantissement");
      cmb_add_type_garantie->addItem("Hypothèque");

      et_caracteristique_garantie = new QTextEdit();
      et_caracteristique_garantie
              ->setPlaceholderText("Caractériques (signes distinctif, valeur approximative, etc...)");

      QPushButton *btn_enregistrer = new QPushButton("Enregistrer");
      QPushButton *btn_quitter = new QPushButton("Fermer");
      QHBoxLayout *lay_btn = new QHBoxLayout();
      QVBoxLayout *lay_dialog = new QVBoxLayout();
      QLabel *lab_en_tete = new QLabel("<h3>Nouvelle garantie</h3>");
      lab_en_tete->setFixedHeight(50);

      QFormLayout *formLayout = new QFormLayout();
      formLayout->addRow("Type : ",cmb_add_type_garantie);
      formLayout->addRow("Caractéristiques : ",et_caracteristique_garantie);

      lay_btn->addSpacing(175);
      lay_btn->addWidget(btn_enregistrer);
      lay_btn->addWidget(btn_quitter);

      lay_dialog->addWidget(lab_en_tete);
      lay_dialog->addLayout(formLayout);
      lay_dialog->addLayout(lay_btn);

      //Connexions
      connect(btn_enregistrer,SIGNAL(clicked(bool)),this,SLOT(enreg_garantie()));
      connect(btn_quitter,SIGNAL(clicked(bool)),this,SLOT(quitter_garantie()));
      return lay_dialog;
}

void FenPrincipale::quitter_garantie()
{
 dialog_nouvelle_garantie->close();
}

void FenPrincipale::enreg_garantie()
{

  if(et_caracteristique_garantie->toPlainText().isEmpty())
  {
      QMessageBox::warning(this,"Enregistrement","Veuillez renseigner tous les champs");
      return;
  }
  else
  {
      QSqlQuery req;
      req.prepare("INSERT INTO Garantie(type,Caracteristiques)"
               "VALUES (:type,:Caracteristiques)");
      req.bindValue(":type",cmb_add_type_garantie->currentText());
      req.bindValue(":Caracteristiques",et_caracteristique_garantie->toPlainText());
      req.exec();
      QMessageBox::information(this,"Information","Enrégistrement réussi");
      et_caracteristique_garantie->setText(" ");
      dialog_nouvelle_garantie->close();

      cmb_enregpret_garantie->clear();

      //Remplir a nouveau le combo

      QSqlQuery reqidgarantie;
      reqidgarantie.exec("SELECT idGarantie,caracteristiques FROM Garantie");
      while(reqidgarantie.next())
      {
          QString idGarantie=reqidgarantie.value("idGarantie").toString();
          QString caracteristique=reqidgarantie.value("caracteristiques").toString();
          cmb_enregpret_garantie->addItem(caracteristique,idGarantie);
      }

  }
}

void FenPrincipale::afficherDialogueNouvelUtilisateur()
{
    authentif_ok=false;
    PASS="";
    dialog_nouvel_utilisateur=new QDialog(this);
    dialog_nouvel_utilisateur->setLayout(ui_enreg_utilisateur_form());
    dialog_nouvel_utilisateur->setFixedSize(450,470);
    dialog_nouvel_utilisateur->setWindowTitle("Utilisateur");
    dialog_nouvel_utilisateur->setStyleSheet(chargerFichier(STYLE_LOGIN_FORM));
    dialog_nouvel_utilisateur->show();

    connect(dialog_nouvel_utilisateur,SIGNAL(finished(int)),this,SLOT(quitterSiNonAuthentifier()));
}

QVBoxLayout* FenPrincipale::ui_enreg_utilisateur_form()
{
    et_utilisateur_login = new QLineEdit();
    et_utilisateur_login->setFixedHeight(25);
    et_utilisateur_password = new QLineEdit();
    et_utilisateur_password->setEchoMode(QLineEdit::Password);
    et_utilisateur_password->setFixedHeight(25);
    et_utilisateur_secretword = new QTextEdit();
    et_utilisateur_secretword->setPlaceholderText("*-Ce texte vous sera "
                                                  "demandé en cas de l'oubli de votre mot de passe.\n");
    et_utilisateur_password_confirm=new QLineEdit();
    et_utilisateur_password_confirm->setEchoMode(QLineEdit::Password);
    et_utilisateur_password_confirm->setToolTip("Saisissez à nouveau le mot de passe");
    et_utilisateur_password_confirm->setPlaceholderText("Saisissez à nouveau");
    et_utilisateur_password_confirm->setFixedHeight(25);


    QVBoxLayout *lay_dialog = new QVBoxLayout();
    QHBoxLayout *lay_avatar = new QHBoxLayout();
    QHBoxLayout *lay_bas = new QHBoxLayout();
    QPushButton *btn_enreg_utilisateur= new QPushButton("Enregistrer");
    QPushButton *btn_fermer_utilisateur= new QPushButton("Fermer");
    QLabel *avatar = new QLabel();
    QLabel *lab_titre = new QLabel("<h3>Nouvel utilisateur</h3>");
    lab_titre->setFixedHeight(50);
    avatar->setPixmap(QPixmap("images/avatar2.png"));
    avatar->setScaledContents(true);
    avatar->setFixedSize(130,130);
    avatar->setAlignment(Qt::AlignVCenter);


    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Identifiant: ",et_utilisateur_login);
    formLayout->addRow("Mot de passe: ",et_utilisateur_password);
    formLayout->addRow("Confirmation : ",et_utilisateur_password_confirm);
    formLayout->addRow("Texte secret : ",et_utilisateur_secretword);

    lay_avatar->addSpacing(150);
    lay_avatar->addWidget(avatar);
    lay_avatar->addSpacing(150);

    lay_bas->addSpacing(150);
    lay_bas->addWidget(btn_enreg_utilisateur);
    lay_bas->addWidget(btn_fermer_utilisateur);

    lay_dialog->addWidget(lab_titre);
    lay_dialog->addLayout(lay_avatar);
    lay_dialog->addSpacing(15);
    lay_dialog->addLayout(formLayout);
    lay_dialog->addSpacing(15);
    lay_dialog->addLayout(lay_bas);

       //Connexions
    connect(btn_enreg_utilisateur,SIGNAL(clicked(bool)),this,SLOT(enreg_utilisateur()));
    connect(btn_fermer_utilisateur,SIGNAL(clicked(bool)),this,SLOT(quitter_utilisateur()));

    return lay_dialog;
}

void FenPrincipale::quitter_utilisateur()
{
       dialog_nouvel_utilisateur->close();
}

void FenPrincipale::enreg_utilisateur()
{
    if(et_utilisateur_login->text().isEmpty()||
            et_utilisateur_password->text().isEmpty()||
            et_utilisateur_password_confirm->text().isEmpty()||
            et_utilisateur_secretword->toPlainText().isEmpty())

    {
        QMessageBox::warning(this,"Attention","Veuillez renseigner tous les champs!");
    }
    else if(et_utilisateur_password->text()!=et_utilisateur_password_confirm->text())
    {
        QMessageBox::warning(this,"Attention","Les deux mots de passe sont différents. Réessayez svp!");
    }
    else
    {
        QSqlQuery req;
        req.prepare("INSERT INTO Utilisateur(login,pass,texteSecret)"
                    "VALUES (:login,:pass,:texteSecret)");
        req.bindValue(":login",et_utilisateur_login->text());
        req.bindValue(":pass",et_utilisateur_password->text());
        req.bindValue(":texteSecret",et_utilisateur_secretword->toPlainText());
        req.exec();
        QMessageBox::information(this,"Information","Inscription réussie");
//        et_utilisateur_login->setText(" ");
//        et_utilisateur_password->setText("");
//        et_utilisateur_password_confirm->setText("");
//        et_utilisateur_secretword->setText(" ");
        USER_NAME = et_utilisateur_login->text();
        PASS = et_utilisateur_password->text();
        dialog_nouvel_utilisateur->close();
    }
}





