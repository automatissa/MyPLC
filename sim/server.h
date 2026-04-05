#pragma once

namespace myplc::sim {

// Start the HTTP simulator dashboard in a background thread.
// After calling this, open http://localhost:<port> in any browser.
// Default port: 8080
void start_server(int port = 8080);

}  // namespace myplc::sim
