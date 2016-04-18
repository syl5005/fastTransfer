/*
FatRat download manager
http://fatrat.dolezel.info

Copyright (C) 2006-2008 Lubos Dolezel <lubos a dolezel.info>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 3 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.

In addition, as a special exemption, Luboš Doležel gives permission
to link the code of FatRat with the OpenSSL project's
"OpenSSL" library (or with modified versions of it that use the; same
license as the "OpenSSL" library), and distribute the linked
executables. You must obey the GNU General Public License in all
respects for all of the code used other than "OpenSSL".
*/

#include "JabberService.h"
#include "fatrat.h"
#include "Proxy.h"
#include "Queue.h"
#include "RuntimeException.h"
#include "Logger.h"
#include "Settings.h"
#include "dbus/DbusImpl.h"

#include <gloox/messagesession.h>
#include <gloox/rostermanager.h>
#include <gloox/disco.h>
#include <gloox/connectionhttpproxy.h>
#include <gloox/connectionsocks5proxy.h>
#include <gloox/connectiontcpclient.h>

#ifdef GLOOX_1_0
#	include <gloox/presence.h>
#	include <gloox/message.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <QSettings>
#include <QTimer>
#include <QSocketNotifier>
#include <QtDebug>

extern QSettings* g_settings;
extern QList<Queue*> g_queues;
extern QReadWriteLock g_queuesLock;

const int SESSION_MINUTES = 10;

JabberService* JabberService::m_instance = 0;

JabberService::JabberService()
	: m_pClient(0), m_pNotifier(0), m_bTerminating(false)
{
	m_instance = this;
	applySettings();
}

JabberService::~JabberService()
{
	if(m_pClient)
	{
		m_bTerminating = true;
		delete m_pClient;
	}
	
	m_instance = 0;
	delete m_pNotifier;
}

void JabberService::socketActivated()
{
	if (m_pClient)
		m_pClient->recv();
}

void JabberService::applySettings()
{
	QString jid, password, resource;
	QUuid proxy;
	int priority;
	bool bChanged = false;
	
	jid = getSettingsValue("jabber/jid").toString();
	password = getSettingsValue("jabber/password").toString();
	priority = getSettingsValue("jabber/priority").toInt();
	proxy = getSettingsValue("jabber/proxy").toString();
	resource = getSettingsValue("jabber/resource").toString();
	
	m_bGrantAuth = getSettingsValue("jabber/grant_auth").toBool();
	m_bRestrictSelf = getSettingsValue("jabber/restrict_self").toBool();
	m_bRestrictPassword = getSettingsValue("jabber/restrict_password_bool").toBool();
	m_strRestrictPassword = getSettingsValue("jabber/restrict_password").toString();
	
	if(jid != m_strJID)
	{
		bChanged = true;
		m_strJID = jid;
	}
	if(password != m_strPassword)
	{
		bChanged = true;
		m_strPassword = password;
	}
	if(priority != m_nPriority)
	{
		bChanged = true;
		m_nPriority = priority;
	}
	if(proxy != m_proxy)
	{
		bChanged = true;
		m_proxy = proxy;
	}
	if(resource != m_strResource)
	{
		bChanged = true;
		m_strResource = resource;
	}
	
	if(getSettingsValue("jabber/enabled").toBool())
	{
		if(!m_pClient)
			run();
		else if(bChanged)
		{
			if(m_pClient)
				m_pClient->disconnect();
		}
	}
	else if(m_pClient)
	{
		m_bTerminating = true;
		if(m_pClient)
		{
			m_pClient->disconnect();
			delete m_pClient;
			m_pClient = 0;
		}
	}
}

void JabberService::run()
{

	gloox::JID jid( (m_strJID + '/' + m_strResource).toStdString());

	Logger::global()->enterLogMessage("Jabber", tr("Connecting..."));

	if (!m_pClient)
	{
		m_pClient = new gloox::Client(jid, m_strPassword.toStdString());
		m_pClient->registerMessageHandler(this);
		m_pClient->registerConnectionListener(this);
		m_pClient->rosterManager()->registerRosterListener(this);
		
		m_pClient->disco()->addFeature(gloox::XMLNS_CHAT_STATES);
		m_pClient->disco()->setIdentity("client", "bot");
		m_pClient->disco()->setVersion("FatRat", VERSION);
	}

	if (m_pNotifier)
	{
		delete m_pNotifier;
		m_pNotifier = 0;
	}

#if defined(GLOOX_0_9)
	m_pClient->setPresence(gloox::PresenceAvailable, m_nPriority);
#elif defined(GLOOX_1_0)
	m_pClient->setPresence(gloox::Presence::Available, m_nPriority);
#endif

	gloox::ConnectionBase* proxy = 0;
	Proxy pdata = Proxy::getProxy(m_proxy);

	if(pdata.nType == Proxy::ProxyHttp)
	{
		gloox::ConnectionHTTPProxy* p;
		proxy = p = new gloox::ConnectionHTTPProxy(m_pClient,
			new gloox::ConnectionTCPClient(m_pClient->logInstance(), pdata.strIP.toStdString(), pdata.nPort),
			m_pClient->logInstance(), m_pClient->server(), m_pClient->port());

		if(!pdata.strUser.isEmpty())
			p->setProxyAuth(pdata.strUser.toStdString(), pdata.strPassword.toStdString());
	}
	else if(pdata.nType == Proxy::ProxySocks5)
	{
		gloox::ConnectionSOCKS5Proxy* p;
		proxy = p = new gloox::ConnectionSOCKS5Proxy(m_pClient,
			new gloox::ConnectionTCPClient(m_pClient->logInstance(), pdata.strIP.toStdString(), pdata.nPort),
			m_pClient->logInstance(), m_pClient->server(), m_pClient->port());

		if(!pdata.strUser.isEmpty())
			p->setProxyAuth(pdata.strUser.toStdString(), pdata.strPassword.toStdString());
	}

	if(proxy != 0)
		m_pClient->setConnectionImpl(proxy);

	if (m_pClient->connect(false))
	{
		int sock = static_cast<gloox::ConnectionTCPClient*>( m_pClient->connectionImpl() )->socket();

		m_pNotifier = new QSocketNotifier(sock, QSocketNotifier::Read, this);
		connect(m_pNotifier, SIGNAL(activated(int)), this, SLOT(socketActivated()));
	}
}

#if defined(GLOOX_1_0)
void JabberService::handleMessage(const gloox::Message& message, gloox::MessageSession* session)
{
	handleMessageGeneric(static_cast<gloox::Stanza*>(const_cast<gloox::Message*>(&message)), session, QString::fromUtf8( message.body().c_str() ));
}
#elif defined(GLOOX_0_9)
void JabberService::handleMessage(gloox::Stanza* stanza, gloox::MessageSession* session)
{
	handleMessageGeneric(stanza, session, QString::fromUtf8( stanza->body().c_str() ));
}
#endif

void JabberService::handleMessageGeneric(gloox::Stanza* stanza, gloox::MessageSession* session, QString message)
{
	ConnectionInfo* conn = getConnection(session, stanza);
	bool bAuthed = conn != 0;
	
	gloox::JID from = stanza->from();
	if(from.resource() == m_strResource.toStdString())
		return;
	
	if(!conn)
	{
		session = new gloox::MessageSession(m_pClient, from);
		session->registerMessageHandler(this);
	}
	
	qDebug() << "Session ID is" << session->threadID().c_str();
	
	if(!bAuthed)
	{
		if(!m_bRestrictPassword)
		{
			if(m_bRestrictSelf)
				bAuthed = from.bare() == m_strJID.toStdString();
			else
				bAuthed = true;
		}
		
		if(bAuthed)
			createConnection(session);
	}
	
	QString msg;
	if(!bAuthed)
	{
		if(!message.startsWith("pass "))
		{
			msg = tr("This is a FatRat remote control bot.\nYou are not authorized. "
					"You may login using a password, if enabled - send:\n\npass yourpassword");
		}
		else
		{
			bool bAccepted = m_bRestrictPassword;
			
			if(m_bRestrictSelf)
			{
				bAccepted &= from.bare() == m_strJID.toStdString();
				if(!bAccepted)
					Logger::global()->enterLogMessage("Jabber", tr("Refusing login for %1 (I am %2)").arg(QString::fromStdString(from.bare())).arg(m_strJID));
			}
			bAccepted &= m_strRestrictPassword == message.mid(5);
			
			if(bAccepted)
			{
				msg = tr("Password accepted, send \"help\" for the list of commands.");
				createConnection(session);
			}
			else
				msg = tr("Password rejected.");
		}
		session->send( qstring2stdstring(msg) );
	}
	else
	{
		if(message == "logout" || message == "exit" || message == "quit")
		{
			msg = tr("Bye.");
			conn->chatState->setChatState(gloox::ChatStateGone);
			session->send( msg.toStdString() );
			m_pClient->disposeMessageSession(session);
			
			Logger::global()->enterLogMessage("Jabber", tr("%1 logged out").arg(from.full().c_str()));
	
			m_connections.removeAll(*conn);
		}
		else
		{
			msg = processCommand(conn, message);
			if(!msg.isEmpty())
				session->send( qstring2stdstring(msg) );
		}
	}
}

QStringList JabberService::parseCommand(QString input, QString* extargs)
{
	QStringList list;
	QString tmp;
	
	int i;
	bool bInStr = false, bInQ = false, bExtArgs = false;
	
	// for oversimplified mobile Jabber clients that don't provide any way of writing \n
	// without sending the message
	input.replace("\\n", "\n");
	
	for(i=0;i<input.size();i++)
	{
		if(input[i] == '\n')
		{
			bExtArgs = true;
			break;
		}
		
		if(input[i] == '"')
		{
			bInQ = bInStr = true;
			continue;
		}
		else if(input[i] != ' ')
			bInStr = true;
		
		if(bInStr)
		{
			if((bInQ && input[i] == '"') || (!bInQ && input[i] == ' '))
			{
				if(!tmp.isEmpty() || bInQ)
				{
					list << tmp;
					tmp.clear();
				}
				bInStr = bInQ = false;
			}
			else
			{
				tmp += input[i];
			}
		}
	}
	
	if(!tmp.isEmpty())
		list << tmp;
	if(bExtArgs && extargs != 0)
	{
		*extargs = input.mid(i+1);
	}
	
	qDebug() << list;
	
	return list;
}

QString JabberService::processCommand(ConnectionInfo* conn, QString cmd)
{
	QString extargs;
	QStringList args = parseCommand(cmd, &extargs);
	QString response;
	
	if(args.isEmpty())
		return response;
	
	try
	{
		if(args[0] == "help")
		{
			response = tr("List of commands:\nqlist - Show list of queues\n"
					"qset - Set current queue ID\n"
					"list - Show transfers of the current queue\n"
					"pauseall/resumeall - Pause/resume all transfers\n"
					"pause/resume/delete - Pause/resume/delete specified transfers\n"
					"logout/quit/exit - Log out\n"
					"\nPass arguments like this: \"resume 1 3 5\", use indexes from the lists\n\n"
					"add/new - Add new transfers\n"
					"This command needs special arguments. See more in the documentation.");
		}
		else if(args[0] == "qlist")
		{
			QReadLocker locker(&g_queuesLock);
			response = tr("List of queues:");
			
			if(!g_queues.isEmpty())
			{
				for(int i=0;i<g_queues.size();i++)
				{
					const Queue::Stats& stats = g_queues[i]->m_stats;
					response += tr("\n#%1 - \"%2\"; %3/%4 active; %5 down, %6 up").arg(i).arg(g_queues[i]->name())
							.arg(stats.active_d+stats.active_u).arg(g_queues[i]->size())
							.arg(formatSize(stats.down, true)).arg(formatSize(stats.up, true));
				}
			}
			else
				response += tr("no queues");
		}
		else if(args[0] == "qset")
		{
			int q = args[1].toInt();
			if(q >= 0 && q < g_queues.size())
			{
				conn->nQueue = q;
				response = tr("OK.");
			}
			else
				response = tr("Invalid queue ID.");
		}
		else if(args[0] == "list")
		{
			QReadLocker locker(&g_queuesLock);
			
			validateQueue(conn);
			response = tr("List of transfers:");
			
			Queue* q = g_queues[conn->nQueue];
			q->lock();
			
			if(q->size())
			{
				for(int i=0;i<q->size();i++)
					response += tr("\n#%1 %2").arg(i).arg(transferInfo(q->at(i)));
			}
			else
				response += tr("no transfers");
			
			q->unlock();
		}
		else if(args[0] == "pauseall" || args[0] == "resumeall")
		{
			Transfer::State state;
			QReadLocker locker(&g_queuesLock);
			validateQueue(conn);
			
			if(args[0] == "resumeall")
				state = Transfer::Active;
			else if(args[0] == "pauseall")
				state = Transfer::Paused;
			
			Queue* q = g_queues[conn->nQueue];
			for(int i=0;i<q->size();i++)
				q->at(i)->setState(state);
		}
		else if(args[0] == "pause" || args[0] == "resume")
		{
			QReadLocker locker(&g_queuesLock);
			validateQueue(conn);
			
			Queue* q = g_queues[conn->nQueue];
			
			Transfer::State state = Transfer::Failed;
			if(args[0] == "pause")
				state = Transfer::Paused;
			else if(args[0] == "resume")
				state = Transfer::Waiting;
			
			response = tr("Set transfer states:");
			
			if (state != Transfer::Failed)
			{
				q->lock();

				for(int i=1;i<args.size();i++)
				{
					int id = args[i].toInt();

					if(id >= 0 && id < q->size())
					{
						q->at(id)->setState(state);
						response += tr("\n#%1 %2").arg(id).arg(transferInfo(q->at(id)));
					}
					else
						response += tr("\n#%1 Invalid transfer ID").arg(id);
				}

				q->unlock();
			}
			else
				response += tr("\nInvalid transfer state");
		}
		else if(args[0] == "remove" || args[0] == "delete")
		{
			QReadLocker locker(&g_queuesLock);
			validateQueue(conn);
			
			Queue* q = g_queues[conn->nQueue];
			QList<int> items;
			
			for(int i=1;i<args.size();i++)
			{
				items << args[i].toInt();
			}
			
			qSort(items);
			
			response = tr("Removing transfers");
			
			for(int i=items.size()-1;i>=0;i--)
			{
				int index = items[i];
				if(index >= 0 && index < q->size())
					q->remove(index);
				else
					response += tr("\n#%1 Invalid transfer ID").arg(index);
			}
		}
		else if(args[0] == "add" || args[0] == "new")
		{
			QReadLocker locker(&g_queuesLock);
			validateQueue(conn);
			
			if(extargs.isEmpty())
				response = tr("Nothing to add");
			else
			{
				if(args.size() < 2 || args[1].startsWith('/'))
					args.insert(1, "auto");
				
				if(args.size() < 3)
				{
					Queue* q = g_queues[conn->nQueue];
					args << q->defaultDirectory();
				}
				
				response = DbusImpl::instance()->addTransfersNonInteractive(extargs, args[2], args[1], conn->nQueue);
				
				if(response.isEmpty())
					response = tr("Transfer(s) added");
			}
		}
		else
		{
			response = tr("Unknown command");
		}
	}
	catch(const RuntimeException& e)
	{
		response = e.what();
	}
	
	qDebug() << response;
	
	return response;
}

QString JabberService::transferInfo(Transfer* t)
{
	int down, up;
	float percent;
	qint64 size, done;
	
	t->speeds(down, up);
	size = t->total();
	done = t->done();
	
	if(size && done)
		percent = 100.f*done/size;
	else
		percent = 0;
	
	return tr("[%2] - \"%3\"; %5 down, %6 up; %7% out of %8").arg(t->state2string(t->state())).arg(t->name())
			.arg(formatSize(down, true)).arg(formatSize(up, true)).arg(percent, 0, 'f', 2).arg(formatSize(size, false));
}

void JabberService::validateQueue(ConnectionInfo* conn)
{
	if(conn->nQueue >= g_queues.size() && conn->nQueue >= 0)
		throw RuntimeException( tr("Invalid queue ID.") );
}

std::string JabberService::qstring2stdstring(QString str)
{
	QByteArray data = str.toUtf8();
	return data.data();
}

JabberService::ConnectionInfo* JabberService::createConnection(gloox::MessageSession* session)
{
	ConnectionInfo info;
	gloox::JID from = session->target();
	info.strJID = QString::fromUtf8( from.full().c_str() );
	info.strThread = QString::fromUtf8( session->threadID().c_str() );
	info.nQueue = (g_queues.isEmpty()) ? -1 : 0;
	info.chatState = new gloox::ChatStateFilter(session);
	info.lastActivity = QDateTime::currentDateTime();
	
	Logger::global()->enterLogMessage("Jabber", tr("New chat session: %1").arg(info.strJID));
	
	info.chatState->registerChatStateHandler(this);
	
	m_connections << info;
	return &m_connections.last();
}

JabberService::ConnectionInfo* JabberService::getConnection(gloox::MessageSession* session, gloox::Stanza* stanza)
{
	ConnectionInfo* rval = 0;
	
	if(!session)
		return 0;
	
	QString remote = (stanza != 0) ? stanza->from().full().c_str() : session->target().full().c_str();
	for(int i=0;i<m_connections.size();i++)
	{
		qDebug() << "Processing" << remote;
		if(m_connections[i].strJID != remote)
			continue;

		if(m_connections[i].strThread.toStdString() == session->threadID() || m_connections[i].strThread.isEmpty())
		{
			const QDateTime currentTime = QDateTime::currentDateTime();
			
			if(m_connections[i].lastActivity.secsTo(currentTime) > SESSION_MINUTES*60)
			{
				m_connections.removeAt(i--);
			}
			else
			{
				rval = &m_connections[i];
				rval->lastActivity = currentTime;
				rval->strThread = session->threadID().c_str();
				break;
			}
		}
	}
	
	return rval;
}

void JabberService::handleChatState(const gloox::JID &from, gloox::ChatStateType state)
{
}

void JabberService::onConnect()
{
	Logger::global()->enterLogMessage("Jabber", tr("Connected"));
	
	gloox::ConnectionTCPBase* base = dynamic_cast<gloox::ConnectionTCPBase*>(m_pClient->connectionImpl());
	
	if(base != 0)
	{
		int sock = base->socket();
		int yes = 1;
		setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof yes);
	}
}

void JabberService::onDisconnect(gloox::ConnectionError e)
{
	QString err = tr("Disconnected:") + ' ';
	
	switch(e)
	{
		case gloox::ConnStreamError:
			err += tr("Stream error"); break;
		case gloox::ConnStreamVersionError:
			err += tr("Stream version error"); break;
		case gloox::ConnStreamClosed:
			err += tr("Stream closed"); break;
		case gloox::ConnProxyAuthRequired:
			err += tr("Proxy authentication required"); break;
		case gloox::ConnProxyAuthFailed:
			err += tr("Proxy authentication failed"); break;
		case gloox::ConnProxyNoSupportedAuth:
			err += tr("The proxy requires an unsupported auth mechanism"); break;
		case gloox::ConnIoError:
			err += tr("I/O error"); break;
		case gloox::ConnParseError:
			err += tr("XML parse error"); break;
		case gloox::ConnConnectionRefused:
			err += tr("Failed to connect"); break;
		case gloox::ConnDnsError:
			err += tr("Failed to resolve the domain name"); break;
		case gloox::ConnOutOfMemory:
			err += tr("Out of memory"); break;
		case gloox::ConnNoSupportedAuth:
			err += tr("The server doesn't provide any supported authentication mechanism"); break;
		case gloox::ConnAuthenticationFailed:
			err += tr("Authentication failed"); break;
		case gloox::ConnUserDisconnected:
			err += tr("The user was disconnected"); break;
		default:
			err += tr("Other reason"); break;
	}
	
	Logger::global()->enterLogMessage("Jabber", err);

	if (m_pNotifier)
	{
		delete m_pNotifier;
		m_pNotifier = 0;
	}

	// reconnect in 3 secs
	if (!m_bTerminating)
		QTimer::singleShot(10*1000, this, SLOT(run()));
}

void JabberService::onResourceBindError(gloox::ResourceBindError error)
{
}

void JabberService::onSessionCreateError(gloox::SessionCreateError error)
{
}
