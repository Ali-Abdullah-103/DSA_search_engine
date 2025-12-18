const express = require('express');
const cors = require('cors');
const { spawn } = require('child_process');
const path = require('path');
const readline = require('readline');

const app = express();
const PORT = 3000;

// Middleware
app.use(cors());
app.use(express.json());
app.use(express.static('public'));

// Path to your C++ server executable
const CPP_SERVER = 'D:\\DSA Project\\DSA_search_engine\\build\\main_server.exe';
console.log('Looking for executable at:', CPP_SERVER);

// Verify the executable exists
const fs = require('fs');
if (!fs.existsSync(CPP_SERVER)) {
    console.error('❌ ERROR: Executable not found at:', CPP_SERVER);
    console.error('Please update the CPP_SERVER path in server.js');
    process.exit(1);
}

// Start the C++ server process
let cppProcess = null;
let isReady = false;
let pendingRequests = [];

function startCppServer() {
    console.log('🔧 Starting C++ search server...');
    
    cppProcess = spawn(CPP_SERVER);
    
    const rl = readline.createInterface({
        input: cppProcess.stdout,
        crlfDelay: Infinity
    });

    // Handle stdout (JSON responses)
    rl.on('line', (line) => {
        console.log('C++ output:', line);
        
        try {
            const response = JSON.parse(line);
            
            // Check if this is the ready signal
            if (response.status === 'ready') {
                isReady = true;
                console.log('✅ C++ server is ready!');
                
                // Process any pending requests
                while (pendingRequests.length > 0) {
                    const req = pendingRequests.shift();
                    req.process();
                }
                return;
            }
            
            // Handle normal responses
            if (pendingRequests.length > 0) {
                const req = pendingRequests.shift();
                req.resolve(response);
            }
        } catch (err) {
            console.error('Failed to parse C++ response:', line);
            if (pendingRequests.length > 0) {
                const req = pendingRequests.shift();
                req.reject(new Error('Invalid JSON response'));
            }
        }
    });

    // Handle stderr (debug messages)
    cppProcess.stderr.on('data', (data) => {
        console.log('C++ log:', data.toString().trim());
    });

    // Handle process exit
    cppProcess.on('close', (code) => {
        console.log(`C++ server exited with code ${code}`);
        isReady = false;
        
        // Reject all pending requests
        while (pendingRequests.length > 0) {
            const req = pendingRequests.shift();
            req.reject(new Error('C++ server crashed'));
        }
    });

    cppProcess.on('error', (err) => {
        console.error('Failed to start C++ server:', err);
        process.exit(1);
    });
}

// Send command to C++ server
function sendCommand(command, query) {
    return new Promise((resolve, reject) => {
        if (!isReady) {
            // Queue the request until server is ready
            pendingRequests.push({
                process: () => sendCommand(command, query).then(resolve).catch(reject),
                resolve,
                reject
            });
            return;
        }

        const requestId = pendingRequests.length;
        pendingRequests.push({ resolve, reject, requestId });
        
        const commandLine = `${command} ${query}\n`;
        console.log('Sending to C++:', commandLine.trim());
        
        cppProcess.stdin.write(commandLine);
        
        // Set timeout
        setTimeout(() => {
            const idx = pendingRequests.findIndex(r => r.requestId === requestId);
            if (idx !== -1) {
                pendingRequests.splice(idx, 1);
                reject(new Error('Request timeout'));
            }
        }, 30000); // 30 second timeout
    });
}

// API Routes
app.get('/api/search', async (req, res) => {
    const query = req.query.q;
    
    if (!query) {
        return res.status(400).json({ error: 'Query parameter "q" is required' });
    }

    try {
        const result = await sendCommand('SEARCH', query);
        res.json(result);
    } catch (err) {
        console.error('Search error:', err);
        res.status(500).json({ error: err.message });
    }
});

app.get('/api/autocomplete', async (req, res) => {
    const query = req.query.q;
    
    if (!query) {
        return res.status(400).json({ error: 'Query parameter "q" is required' });
    }

    try {
        const result = await sendCommand('AUTOCOMPLETE', query);
        res.json(result);
    } catch (err) {
        console.error('Autocomplete error:', err);
        res.status(500).json({ error: err.message });
    }
});

// Health check endpoint
app.get('/api/health', (req, res) => {
    res.json({ 
        status: isReady ? 'ready' : 'loading',
        pendingRequests: pendingRequests.length
    });
});

// Graceful shutdown
process.on('SIGINT', () => {
    console.log('\n🛑 Shutting down...');
    if (cppProcess) {
        cppProcess.stdin.write('EXIT\n');
        setTimeout(() => {
            cppProcess.kill();
            process.exit(0);
        }, 1000);
    } else {
        process.exit(0);
    }
});

// Start the C++ server
startCppServer();

// Start Express server
app.listen(PORT, () => {
    console.log(`🚀 Node.js server running at http://localhost:${PORT}`);
    console.log(`⏳ Waiting for C++ server to load indices...`);
});