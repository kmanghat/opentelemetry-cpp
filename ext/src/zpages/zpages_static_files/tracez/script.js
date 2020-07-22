window.onload = () => refreshData();

const latencies = [
  '>0s', '>10&#181s', '>100&#181s',
  '>1ms', '>10ms', '>100ms',
  '>1s', '>10s', '>100s',
];

const statusCodesDescriptions = {
  'OK': 'The operation completed successfully.',
  'CANCELLED': 'The operation was cancelled (typically by the caller).',
  'UNKNOWN': `Unknown error. An example of where this error may be returned is if a Status value received
   	from another address space belongs to an error-space that is not known in this address space.
   	Also errors raised by APIs that do not return enough error information may be converted to
   	this error.`,
  'INVALID_ARGUMENT': `Client specified an invalid argument. Note that this differs from FAILED_PRECONDITION.
   	INVALID_ARGUMENT indicates arguments that are problematic regardless of the state of the
   	system (e.g., a malformed file name).`,
  'DEADLINE_EXCEEDED': `Deadline expired before operation could complete. For operations that change the state of the
   	system, this error may be returned even if the operation has completed successfully. For
   	example, a successful response from a server could have been delayed long enough for the
   	deadline to expire.`,
  'NOT_FOUND' : 'Some requested entity (e.g., file or directory) was not found.',
  'ALREADY_EXISTS': 'Some entity that we attempted to create (e.g., file or directory) already exists.',
  'PERMISSION_DENIED': `The caller does not have permission to execute the specified operation. PERMISSION_DENIED
   	must not be used for rejections caused by exhausting some resource (use RESOURCE_EXHAUSTED
   	instead for those errors). PERMISSION_DENIED must not be used if the caller cannot be
   	identified (use UNAUTHENTICATED instead for those errors).`,
  'RESOURCE_EXHAUSTED': `Some resource has been exhausted, perhaps a per-user quota, or perhaps the entire file system
   	is out of space.`,
  'FAILED_PRECONDITION': `Operation was rejected because the system is not in a state required for the operation's
   	execution. For example, directory to be deleted may be non-empty, an rmdir operation is
   	applied to a non-directory, etc.`,
  'ABORTED': `The operation was aborted, typically due to a concurrency issue like sequencer check
   	failures, transaction aborts, etc`,
  'OUT_OF_RANGE': `Operation was attempted past the valid range. E.g., seeking or reading past end of file.`,
  'UNIMPLEMENTED': 'Operation is not implemented or not supported/enabled in this service.',
  'INTERNAL': `Internal errors. Means some invariants expected by underlying system has been broken. If you
	see one of these errors, something is very broken.`,
  'UNAVAILABLE': `The service is currently unavailable. This is a most likely a transient condition and may be
   	corrected by retrying with a backoff.`,
  'DATA_LOSS': 'Unrecoverable data loss or corruption.',
  'UNAUTHENTICATED': 'The request does not have valid authentication credentials for the operation.',
};

// Standard categories when checking span details
const detailCols = ['spanid', 'parentid', 'traceid', 'start', 'description']; // Columns error, running, and latency spans all share
const numCols = ['duration']; // Categories to change to num
const dateCols = ['start']; // Categories to change to date
const clickCols = ['error', 'running']; // Non-latency clickable cols
const updateLastRefreshStr = () => document.getElementById('lastUpdateTime').innerHTML = new Date().toLocaleString(); // update

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
      {'sz': 'sm', 'repeats': 4},
      {'sz': 'md', 'repeats': 1},
      {'sz': 'sm', 'repeats': 1},
     ],
    'headings': [...detailCols, 'status'],
  },
  'running': {
    'url': base_endpt + 'running/',
    'html_id': 'name_type_detail_table',
    'sizing': [
      {'sz': 'sm', 'repeats': 4},
      {'sz': 'md', 'repeats': 1},
     ],
    'headings': detailCols,
    'status': 'pending',
  },
  'latency': {
    'url': base_endpt + 'latency/',
    'html_id': 'name_type_detail_table',
    'sizing': [
      {'sz': 'sm', 'repeats': 4},
      {'sz': 'md', 'repeats': 1},
      {'sz': 'sm', 'repeats': 1},
     ],
    'headings': [...detailCols, ...numCols],
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

const isLatency = group => !(new Set(clickCols).has(group)); 
const hasCallback = col => !isLatency(col); // Non-latency cb columns
const hideHeader = h => new Set([...clickCols, 'name']).has(h); // Headers to not show render twice

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

const getLatencyCells = spans => spans['latency'].map(span => `<td class='click'>${span}</td>`).join('');

// Create cell based on what header we want to render
const getCell = (h, span) => (h === 'latency') ? getLatencyCells(span)
  : `<td ${hasCallback(h) ? (`class='click'
          onclick="overwriteDetailedTable('${h}', '${span['name']}')"`)
      : ''}>` + `${span[h]}</td>`;

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

// Overwrites a table on the DOM based on the group and also
// changes the subheader, since this a looking at sampled spans
function overwriteDetailedTable(group, name) {
  if (isLatency(group)) overwriteTable('latency', group + '/' + name);
  else overwriteTable(group, name);
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

