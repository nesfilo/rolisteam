/************************************************************************
*   Copyright (C) 2007 by Romain Campioni                               *
*   Copyright (C) 2009 by Renaud Guezennec                              *
*   Copyright (C) 2010 by Joseph Boudou                                 *
*                                                                       *
*     http://www.rolisteam.org/                                         *
*                                                                       *
*   rolisteam is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published   *
*   by the Free Software Foundation; either version 2 of the License,   *
*   or (at your option) any later version.                              *
*                                                                       *
*   This program is distributed in the hope that it will be useful,     *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of      *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       *
*   GNU General Public License for more details.                        *
*                                                                       *
*   You should have received a copy of the GNU General Public License   *
*   along with this program; if not, write to the                       *
*   Free Software Foundation, Inc.,                                     *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           *
*************************************************************************/


#include "network/networkmanager.h"

#include <QTcpSocket>
//#include <QMessageBox>
#include "network/networklink.h"
#include "data/person.h"
#include "data/player.h"
#include "network/connectionprofile.h"

#define second 1000
NetworkLink* ClientManager::m_networkLinkToServer = nullptr;
/*****************
 * ClientManager *
 *****************/
ClientManager::ClientManager(ConnectionProfile* connection)
    : QObject(),m_connectionProfile(connection),m_disconnectAsked(false),m_connectionState(DISCONNECTED),m_localPlayer(nullptr)
{
    m_reconnect = new QTimer(this);
    m_playersList = PlayersList::instance();

    m_networkLinkToServer = new NetworkLink(connection);


    connect(this, SIGNAL(sendData(char *, quint32, NetworkLink *)),m_networkLinkToServer, SLOT(sendData(char *, quint32, NetworkLink *)));
    connect(m_networkLinkToServer, SIGNAL(disconnected(NetworkLink *)),this, SLOT(endingNetworkLink(NetworkLink *)));
    connect(m_networkLinkToServer,SIGNAL(readDataReceived(quint64,quint64)),this,SIGNAL(dataReceived(quint64,quint64)));
    connect(m_networkLinkToServer,SIGNAL(errorMessage(QString)),this,SIGNAL(errorOccur(QString)));

    m_connecting = new QState();
    m_connected= new QState();
    m_authentified= new QState();
    m_error= new QState();
    m_disconnected= new QState();

    connect(m_connected,&QAbstractState::entered,[=]()
    {
        setConnectionState(CONNECTED);
    });
    connect(m_connecting,&QAbstractState::entered,[=]()
    {
        setConnectionState(CONNECTING);
        emit isConnecting();

    });
    connect(m_disconnected,&QAbstractState::entered,[=]()
    {
        setConnectionState(DISCONNECTED);
        emit isDisconnected();
    });
    connect(m_authentified,&QAbstractState::entered,[=]()
    {
        setConnectionState(AUTHENTIFIED);
        emit isAuthentified();
        PlayersList*  playerList = PlayersList::instance();
        if(!m_connectionProfile->isGM())
        {
            playerList->addLocalCharacter(m_connectionProfile->getCharacter());
        }
        playerList->sendOffLocalPlayerInformations();
        playerList->sendOffFeatures(m_connectionProfile->getPlayer());
    });

    m_states.addState(m_connecting);
    m_states.addState(m_connected);
    m_states.addState(m_authentified);
    m_states.addState(m_disconnected);
    m_states.addState(m_error);

    m_states.setInitialState(m_disconnected);

    m_disconnected->addTransition(m_networkLinkToServer,SIGNAL(connecting()),m_connecting);
    m_connecting->addTransition(m_networkLinkToServer,SIGNAL(connected()),m_connected);
    m_connected->addTransition(m_networkLinkToServer,SIGNAL(authentificationSuccessed()),m_authentified);
    //m_connected->addTransition(m_networkLinkToServer,SIGNAL(authentificationFail()),m_disconnected);

    m_states.addTransition(m_networkLinkToServer,SIGNAL(error()),m_error);
    m_states.addTransition(m_networkLinkToServer,SIGNAL(disconnected()),m_disconnected);

    connect(&m_states,SIGNAL(started()),this,SIGNAL(isReady()));

    m_states.start();

}

ClientManager::~ClientManager()
{
    delete m_reconnect;
}

Player* ClientManager::getLocalPlayer()
{
    return m_localPlayer;
}
bool ClientManager::startConnection()
{
    // Server setup
    if (m_connectionProfile->isServer())
    {
        m_connectionProfile->setAddress("localhost");
    }
    startConnectionToServer();
    return true;
}

void ClientManager::startConnectionToServer()
{
    if(NULL==m_networkLinkToServer)
    {
        m_networkLinkToServer->connectTo();
    }
    else
    {
        m_networkLinkToServer->setConnection(m_connectionProfile);
        m_networkLinkToServer->connectTo();
    }
}

void ClientManager::sendMessage(char* data, quint32 size, NetworkLink* but)
{
    NetworkMessageHeader header;
    memcpy((char*)&header,data,sizeof(NetworkMessageHeader));

    //client
    if(NULL!=m_networkLinkToServer)
    {
        m_networkLinkToServer->sendDataSlot(data,size,but);
    }
}


void ClientManager::addNetworkLink(NetworkLink* networkLink)
{
 /*   if((NULL!=networkLink)&&(m_networkLinkList.contains(networkLink)))
    {
        qDebug() << "#################\nNetworkLink is already inside;";
    }

    if((NULL!=networkLink)&&(!m_networkLinkList.contains(networkLink)))
    {
        m_networkLinkList.append(networkLink);
        connect(this, SIGNAL(sendData(char *, quint32, NetworkLink *)),networkLink, SLOT(sendData(char *, quint32, NetworkLink *)));
        connect(networkLink, SIGNAL(disconnected(NetworkLink *)),this, SLOT(endingNetworkLink(NetworkLink *)));
        connect(networkLink,SIGNAL(readDataReceived(quint64,quint64)),this,SIGNAL(dataReceived(quint64,quint64)));
        emit linkAdded(networkLink);
    }*/
}



void ClientManager::endingNetworkLink(NetworkLink * link)
{

    if(!m_disconnectAsked)
    {
        link->deleteLater();

        emit linkDeleted(link);
    }


    if (!m_connectionProfile->isServer())
    {
        if(link==m_networkLinkToServer)
        {
            setConnectionState(DISCONNECTED);
            emit notifyUser(tr("Connection with the Remote Server has been lost."));
            m_networkLinkToServer = NULL;
            m_playersList->cleanListButLocal();
        }


       /* if(!m_disconnectAsked)
        {
            m_dialog->startTimer();
            m_dialog->show();
        }*/
    }
  /*  else
    {
        if (link == NULL)
        {
			qWarning() << tr("NULL NetworkLink pointer (NetworkManager::finDeNetworkLink).");
            return;
        }

        int i = m_networkLinkList.indexOf(link);
        if (i < 0)
        {
			qWarning()<< tr("Unknown thread joined, (finDeNetworkLink - NetworkManager.cpp)");
            return;
        }
        m_networkLinkList.removeAt(i);
  //  }*/
}
void ClientManager::disconnectAndClose()
{
    m_disconnectAsked = true;
    m_networkLinkToServer->disconnectAndClose();
    emit notifyUser(tr("Connection to the server has been closed."));
    m_playersList->cleanListButLocal();
    delete m_networkLinkToServer;
    m_networkLinkToServer=NULL;
    setConnectionState(DISCONNECTED);
}
bool ClientManager::isServer() const
{
    return false;
}
bool ClientManager::isConnected() const
{
    qDebug() << m_connectionState;
    switch (m_connectionState)
    {
    case DISCONNECTED:
    case CONNECTING:
        return false;
    case CONNECTED:
    case AUTHENTIFIED:
        return true;
    }
}

void ClientManager::setConnectionState(ConnectionState state)
{
    if(m_connectionState!=state)
    {
        m_connectionState=state;
        emit connectionStateChanged(m_connectionState);
    }
}
NetworkLink* ClientManager::getLinkToServer()
{
    return m_networkLinkToServer;
}
void ClientManager::setConnectionProfile(ConnectionProfile* profile)
{
    m_connectionProfile = profile;
    m_networkLinkToServer = new NetworkLink(m_connectionProfile);
    if(!m_connectionProfile->isServer())
    {
        m_hbSender = new heartBeatSender(this);
        if(nullptr != m_localPlayer)
        {
            m_hbSender->setIdLocalUser(m_localPlayer->getUuid());
        }
    }
}