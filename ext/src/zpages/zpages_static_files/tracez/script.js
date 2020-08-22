window.onload = () => refreshData();

const latencies = [
  '>0s', '>10&#181s', '>100&#181s',
  '>1ms', '>10ms', '>100ms',
  '>1s', '>10s', '>100s',
];

// Latency info is returned as an array, so they need to be parsed accordingly
const getLatencyCell = (span, i, h) => `<td class='click'
      onclick="overwriteDetailedTable(${i}, '${span['name']}')">${span[h][i]}</td>`;

// Pretty print a cell with a map
const getKeyValueCell = (span, i, h) => `<td><pr><code>
	${JSON.stringify(span[h][i], null, 2)}
	</code></pre></td>`;

// Standard categories when checking span details
const idCols = ['spanid', 'parentid', 'traceid']
const detailCols = ['description']; // Columns error, running, and latency spans all share
const dateCols = ['start']; // Categories to change to date
const numCols = ['duration']; // Categories to change to num
const clickCols = ['error', 'running']; // Non-latency clickable cols
const arrayCols = { 
  'latency': getLatencyCell,
  'events': getKeyValueCell,
  'attributes': getKeyValueCell
};

const base_endpt = '/tracez/get/'; // For making GET requests
// Maps table types to their approporiate formatting
const tableFormatting = {
  'all': {
    'url': base_endpt + 'aggregations',
    'html_id': 'overview_table',
    'sizing': [
      {'sz': 'md', 'repeats': 1},
      {'sz': 'sm', 'repeats': 11},
    ],
    'headings': ['name', ...clickCols, 'latency'],
    'cell_headings': ['name', ...clickCols, ...latencies],
  },
  'error': {
    'url': base_endpt + 'error/',
    'html_id': 'name_type_detail_table',
    'sizing': [
      {'sz': 'sm', 'repeats': 5},
      {'sz': 'md', 'repeats': 1},
     ],
    'headings': [...idCols, ...dateCols, 'status', ...detailCols],
    'has_subheading': true,
  },
  'running': {
    'url': base_endpt + 'running/',
    'html_id': 'name_type_detail_table',
    'sizing': [
      {'sz': 'sm', 'repeats': 4},
      {'sz': 'md', 'repeats': 1},
     ],
    'headings': [...idCols, ...dateCols, ...detailCols],
    'has_subheading': true,
    'status': 'pending',
  },
  'latency': {
    'url': base_endpt + 'latency/',
    'html_id': 'name_type_detail_table',
    'sizing': [
      {'sz': 'sm', 'repeats': 5},
      {'sz': 'md', 'repeats': 1},
     ],
    'headings': [...idCols, ...dateCols, ...numCols, ...detailCols],
    'has_subheading': true,
    'status': 'ok'
  }
};
const getFormat = group => tableFormatting[group];


// Getters using formatting config variable
const getURL = group => getFormat(group)['url'];
const getHeadings = group => getFormat(group)['headings'];
const getCellHeadings = group => 'cell_headings' in getFormat(group)
	? getFormat(group)['cell_headings'] : getHeadings(group); 
const getSizing = group => getFormat(group)['sizing'];
const getStatus = group => isLatency(group) ? 'ok' : getFormat(group)['status'];
const getHTML = group => getFormat(group)['html_id'];

const isDate = col => new Set(dateCols).has(col);
const isLatency = group => !(new Set(clickCols).has(group)); // non latency clickable cols, change to include latency?
const isArrayCol = group => (new Set(Object.keys(arrayCols)).has(group));
const hasCallback = col => new Set(clickCols).has(col); //!isLatency(col); // Non-latency cb columns
const hideHeader = h => new Set([...clickCols, 'name']).has(h); // Headers to not show render twice
const hasSubheading = group => isLatency(group) || 'has_subheading' in getFormat(group); 
const hasStatus = group => isLatency(group) || 'status' in getFormat(group);

const toTitlecase = word => word.charAt(0).toUpperCase() + word.slice(1);
const updateLastRefreshStr = () => document.getElementById('lastUpdateTime').innerHTML = new Date().toLocaleString(); // update

const getStatusHTML = group => !hasStatus(group) ? ''
  : `All of these spans have status code ${getStatus(group)}`;

// Returns an HTML string that handlles width formatting
// for a table group
const tableSizing = group => '<colgroup>'
  + getSizing(group).map(sz =>
       	(`<col class='${sz['sz']}'></col>`).repeat(sz['repeats']))
       	.join('')
  + '</colgroup>';

// Returns an HTML string for a table group's headings,
// hiding headings where needed
const tableHeadings = group => '<tr>'
    + getCellHeadings(group).map(h => `<th>${(hideHeader(h) ? '' : h)}</th>`).join('')
    + '</tr>';

// Returns an HTML string, which represents the formatting for
// the entire header for a table group. This doesn't change, and
// includes the width formatting and the actual table headers
const tableHeader = group => tableSizing(group) + tableHeadings(group);

// Return formatting for an array-based value based on its header
const getArrayCells = (h, span) => span[h].length
  ? (span[h].map((_, i) => arrayCols[h](span, i, h))).join('')
  : 'Empty';

const EmptyContent = () => `<span class='empty'>(not set)</span>`

// Convert cells to Date strings if needed
const getCellContent = (h, span) => {
  if (!isDate(h)) return (span[h] !== '') ? span[h] : EmptyContent();
  return new Date(span[h] / 1000000).toJSON();
};

// Create cell based on what header we want to render
const getCell = (h, span) => (isArrayCol(h)) ? getArrayCells(h, span)
  : `<td ${hasCallback(h) ? (`class='click'
          onclick="overwriteDetailedTable('${h}', '${span['name']}')"`)
      : ''}>` + `${getCellContent(h, span)}</td>`;

// Returns an HTML string with for a span's aggregated data
// while columns are ordered according to its table group
const tableRow = (group, span) => '<tr>'
    + getHeadings(group).map(h => getCell(h, span)).join('')
    + '</tr>';

// Returns an HTML string from all the data given as
// table rows, with each row being a group of spans by name
const tableRows = (group, data) => data.map(span => tableRow(group, span)).join('');

// Overwrite a table on the DOM based on the group given by adding
// its headers and fetching data for its url
function overwriteTable(group, url_end = '') {
  console.log(getURL(group) + url_end);
  fetch(getURL(group) + url_end).then(res => res.json())
    .then(data => {
      console.log(data);
       document.getElementById(getHTML(group))
          .innerHTML = tableHeader(group)
            + tableRows(group, data);
      })
    .catch(err => console.log(err));
};

// Adds a title subheading where needed
function updateSubheading(group, name) {
  if (hasSubheading(group)) {
    document.getElementById(getHTML(isLatency(group) ? 'latency' : group) + '_header')
        .innerHTML = `<h2>${name}<br>
            ${(isLatency(group) ? `${latencies[group]} Bucket` : toTitlecase(group))}
            Spans</h2><i>Showing sampled span details (up to 5).
            ${getStatusHTML(group)}</i><br><br>`;
  }
};

// Overwrites a table on the DOM based on the group and also
// changes the subheader, since this a looking at sampled spans
function overwriteDetailedTable(group, name) {
  if (isLatency(group)) overwriteTable('latency', group + '/' + name);
  else overwriteTable(group, name);
  updateSubheading(group, name);
};

// Append to a table on the DOM based on the group given
function addToTable(group, url_end = '') {
  fetch(getURL(group) + url_end).then(res => res.json())
    .then(data => {
      const rowsStr = tableRows(group, data);
      if (!rowsStr) console.log(`No rows added for ${group} table`);
      document.getElementById(getHTML(group))
        .getElementsByTagName('tbody')[0]
        .innerHTML += rowsStr;
      })
    .catch(err => console.log(err));
};

const refreshData = () => {
  updateLastRefreshStr();
  overwriteTable('all');
};

