#ifndef SELECTCONNECTIONPROFILEDIALOG_H
#define SELECTCONNECTIONPROFILEDIALOG_H

#include <QDialog>
#include "data/player.h"
#include "connectionprofile.h"
#include <QSettings>

#include <QAbstractListModel>

namespace Ui {
class SelectConnectionProfileDialog;
}


/**
 * @brief The ProfileModel class stores all users profile. It is read from settings. User can add, update or remove profile.
 */
class ProfileModel : public QAbstractListModel
{
    Q_OBJECT
public:
    /**
    * @brief ProfileModel
    */
   ProfileModel(QString version);
   /**
    * @brief ~ProfileModel
    */
   virtual ~ProfileModel();
   /**
    * @brief rowCount
    * @param parent
    * @return
    */
   virtual int rowCount(const QModelIndex &parent) const;
   /**
    * @brief data
    * @param index
    * @param role
    * @return
    */
   virtual QVariant data(const QModelIndex &index, int role) const;
   /**
    * @brief headerData
    * @param section
    * @param orientation
    * @param role
    * @return
    */
   QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole);
   /**
   * Load informations from the previous rolisteam's execution
   */
   void readSettings(QSettings & settings);

   /**
   * Save parameters for next executions.
   */
   void writeSettings(QSettings & settings);

   Qt::ItemFlags flags(const QModelIndex & index);
   void removeProfile(ConnectionProfile* profile);

   ConnectionProfile* getProfile(const QModelIndex&);

   void cloneProfile(const QModelIndex& index);

   int indexOf(ConnectionProfile *tmp);
   ConnectionProfile *getProfile(int index);
public slots:
   /**
    * @brief ProfileModel::appendProfile
    */
   void appendProfile();
   void appendProfile(ConnectionProfile *profile);
private:
    QList<ConnectionProfile*> m_connectionProfileList;
    QString m_version;
};

/**
 * @brief The SelectConnectionProfileDialog class is the dialog box shown at starting or when the connection is lost.
 */
class SelectConnectionProfileDialog : public QDialog
{
	Q_OBJECT

public:
    /**
     * @brief SelectConnectionProfileDialog
     * @param parent
     */
    explicit SelectConnectionProfileDialog(QString version, QWidget *parent = 0);
    /**
     * @brief ~SelectConnectionProfileDialog
     */
    virtual	~SelectConnectionProfileDialog();

    /**
     * @brief getSelectedProfile
     * @return
     */
    ConnectionProfile* getSelectedProfile();

    /**
    * Load informations from the previous rolisteam's execution
    */
    void readSettings(QSettings & settings);

    /**
    * Save parameters for next executions.
    */
    void writeSettings(QSettings & settings);

    /**
     * @brief setArgumentProfile
     * @param host
     * @param port
     * @param password
     */
    void setArgumentProfile(QString host,int port,QString password);
    /**
     * @brief updateGUI
     */
    void updateGUI();
signals:
    /**
     * @brief tryConnection
     */
    void tryConnection();
public slots:
    /**
     * @brief removeProfile
     */
    void removeProfile();
    /**
     * @brief updateProfile
     */
    void updateProfile();
    /**
     * @brief setCurrentProfile
     */
    void setCurrentProfile(QModelIndex );
    void connectTo();
    void connectToIndex(QModelIndex index);
    void openImage();
    void errorOccurs(QString);
    void checkConnection();
    void disconnection();
private:
	Ui::SelectConnectionProfileDialog *ui;
    ProfileModel* m_model;
    ConnectionProfile* m_currentProfile;
    QString m_version;
    QString m_avatarUri;
};

#endif // SELECTCONNECTIONPROFILEDIALOG_H
