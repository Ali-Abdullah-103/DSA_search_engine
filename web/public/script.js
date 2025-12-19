const searchInput = document.getElementById('searchInput');
const searchBtn = document.getElementById('searchBtn');
const autocompleteDiv = document.getElementById('autocomplete');
const resultsDiv = document.getElementById('results');
const loadingDiv = document.getElementById('loading');

const API_BASE = 'http://localhost:3000/api';

let autocompleteTimeout;

// Search function
async function performSearch(query) {
    if (!query.trim()) return;

    // Hide autocomplete
    autocompleteDiv.classList.remove('show');
    autocompleteDiv.innerHTML = '';

    // Show loading
    loadingDiv.classList.remove('hidden');
    resultsDiv.innerHTML = '';

    try {
        const response = await fetch(`${API_BASE}/search?q=${encodeURIComponent(query)}`);
        const data = await response.json();

        const resultsCount = data.results ? data.results.length : 0;
        if (data.search_time_ms) {
            document.getElementById("searchTime").innerText = 
                `About ${resultsCount} results (${data.search_time_ms} ms)`;
        } else {
            document.getElementById("searchTime").innerText = 
                `About ${resultsCount} results`;
        }

        loadingDiv.classList.add('hidden');

        if (data.error) {
            resultsDiv.innerHTML = `<div class="no-results">Error: ${data.error}</div>`;
            return;
        }

        displayResults(data.results, query);
    } catch (error) {
        loadingDiv.classList.add('hidden');
        resultsDiv.innerHTML = `<div class="no-results">Error connecting to server. Make sure the server is running.</div>`;
        console.error('Search error:', error);
    }
}

// Display search results
function displayResults(results, query) {
    if (!results || results.length === 0) {
        resultsDiv.innerHTML = `<div class="no-results">No results found for "${query}"</div>`;
        return;
    }

    let html = `<div class="results-header">About ${results.length} results</div>`;

    results.forEach((result, index) => {
        const title = result.title || 'Untitled Document';
        const url = result.url || '#';
        const score = result.score ? result.score.toFixed(4) : '0.0000';

        html += `
            <div class="result-item">
                <a href="${url}" target="_blank" class="result-title">${escapeHtml(title)}</a>
                <div class="result-url">
                    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24">
                        <path d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm-2 15l-5-5 1.41-1.41L10 14.17l7.59-7.59L19 8l-9 9z"/>
                    </svg>
                    ${escapeHtml(url)}
                    <span class="result-score">Score: ${score}</span>
                </div>
            </div>
        `;
    });

    resultsDiv.innerHTML = html;
}

// Autocomplete function
async function fetchAutocomplete(query) {
    if (!query.trim()) {
        autocompleteDiv.classList.remove('show');
        autocompleteDiv.innerHTML = '';
        return;
    }

    try {
        const response = await fetch(`${API_BASE}/autocomplete?q=${encodeURIComponent(query)}`);
        const data = await response.json();

        if (data.error) {
            console.error('Autocomplete error:', data.error);
            return;
        }

        displayAutocomplete(data.suggestions);
    } catch (error) {
        console.error('Autocomplete error:', error);
    }
}

// Display autocomplete suggestions
function displayAutocomplete(suggestions) {
    if (!suggestions || suggestions.length === 0) {
        autocompleteDiv.classList.remove('show');
        autocompleteDiv.innerHTML = '';
        return;
    }

    let html = '';
    suggestions.forEach(suggestion => {
        html += `
            <div class="autocomplete-item" data-suggestion="${escapeHtml(suggestion)}">
                <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24">
                    <path d="M15.5 14h-.79l-.28-.27A6.471 6.471 0 0 0 16 9.5 6.5 6.5 0 1 0 9.5 16c1.61 0 3.09-.59 4.23-1.57l.27.28v.79l5 4.99L20.49 19l-4.99-5zm-6 0C7.01 14 5 11.99 5 9.5S7.01 5 9.5 5 14 7.01 14 9.5 11.99 14 9.5 14z"/>
                </svg>
                ${escapeHtml(suggestion)}
            </div>
        `;
    });

    autocompleteDiv.innerHTML = html;
    autocompleteDiv.classList.add('show');

    // Add click handlers to autocomplete items
    document.querySelectorAll('.autocomplete-item').forEach(item => {
        item.addEventListener('click', function() {
            const suggestion = this.getAttribute('data-suggestion');
            searchInput.value = suggestion;
            performSearch(suggestion);
        });
    });
}

// Escape HTML to prevent XSS
function escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

// Event Listeners
searchInput.addEventListener('input', function() {
    clearTimeout(autocompleteTimeout);
    const query = this.value;
    
    if (query.trim()) {
        autocompleteTimeout = setTimeout(() => {
            fetchAutocomplete(query);
        }, 300); // Debounce for 300ms
    } else {
        autocompleteDiv.classList.remove('show');
        autocompleteDiv.innerHTML = '';
    }
});

searchInput.addEventListener('keypress', function(e) {
    if (e.key === 'Enter') {
        performSearch(this.value);
    }
});

searchBtn.addEventListener('click', function() {
    performSearch(searchInput.value);
});

// Hide autocomplete when clicking outside
document.addEventListener('click', function(e) {
    if (!searchInput.contains(e.target) && !autocompleteDiv.contains(e.target)) {
        autocompleteDiv.classList.remove('show');
    }
});

// Focus on search input on page load
searchInput.focus();