/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file WhisperHost.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <mutex>
#include <array>
#include <set>
#include <memory>
#include <utility>
#include <libdevcore/RLP.h>
#include <libdevcore/Guards.h>
#include <libdevcrypto/SHA3.h>
#include "Common.h"
#include "WhisperPeer.h"
#include "Interface.h"

namespace dev
{
namespace shh
{

class WhisperHost: public HostCapability<WhisperPeer>, public Interface
{
	friend class WhisperPeer;

public:
	WhisperHost();
	virtual ~WhisperHost();

	unsigned protocolVersion() const { return 0; }

	virtual void inject(Message const& _m, WhisperPeer* _from = nullptr);

	virtual unsigned installWatch(MessageFilter const& _filter);
	virtual unsigned installWatch(h256 _filterId);
	virtual void uninstallWatch(unsigned _watchId);
	virtual h256s peekWatch(unsigned _watchId) const { dev::Guard l(m_filterLock); try { return m_watches.at(_watchId).changes; } catch (...) { return h256s(); } }
	virtual h256s checkWatch(unsigned _watchId) { dev::Guard l(m_filterLock); h256s ret; try { ret = m_watches.at(_watchId).changes; m_watches.at(_watchId).changes.clear(); } catch (...) {} return ret; }

	virtual Message message(h256 _m) const { try { dev::ReadGuard l(x_messages); return m_messages.at(_m); } catch (...) { return Message(); } }

	virtual void sendRaw(bytes const& _payload, Topic _topic, unsigned _ttl) { inject(Message(time(0) + _ttl, _ttl, _topic, _payload)); }

private:
	void streamMessage(h256 _m, RLPStream& _s) const;

	void noteChanged(h256 _messageHash, h256 _filter);

	mutable dev::SharedMutex x_messages;
	std::map<h256, Message> m_messages;

	mutable dev::Mutex m_filterLock;
	std::map<h256, InstalledFilter> m_filters;
	std::map<unsigned, ClientWatch> m_watches;
};

}
}
