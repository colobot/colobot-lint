/* sorting functions */

function compareLocations(loc1, loc2)
{
    var loc1Str = $.text([loc1]);
    var loc2Str = $.text([loc2]);

    var loc1Split = loc1Str.split(':');
    var loc2Split = loc2Str.split(':');

    // compare filenames first
    var filenameCompare = loc1Split[0].localeCompare(loc2Split[0]);
    if (filenameCompare != 0)
        return filenameCompare;

    // compare line numbers second
    var loc1LineNum = parseInt(loc1Split[1], 10)
    var loc2LineNum = parseInt(loc2Split[1], 10)
    if (loc1LineNum > loc2LineNum)
        return 1;
    else if (loc1LineNum < loc2LineNum)
        return -1;

    return 0;
}

var SEVERITY_ORDER = [
    'error',
    'warning',
    'information',
    'style'
];

function compareSeverity(severity1, severity2)
{
    var severity1Str = $.text([severity1]);
    var severity2Str = $.text([severity2]);

    var result = 0;
    SEVERITY_ORDER.forEach(
        function(severityOrder)
        {
            if (severity1Str == severityOrder &&
                severity2Str == severityOrder)
            {
                result = 0;
                return;
            }
            else if (severity1Str == severityOrder)
            {
                result = 1;
                return;
            }
            else if (severity2Str == severityOrder)
            {
                result = -1;
                return;
            }
        }
    );

    return result;
}

function compareGeneric(a, b)
{
    return $.text([a]).localeCompare($.text([b]));
}

function createCompareFunction(inverse, columnClass)
{
    var whichFunction;

    if (columnClass == 'location')
        whichFunction = compareLocations;
    else if (columnClass == 'severity')
        whichFunction = compareSeverity;
    else
        whichFunction = compareGeneric;

    return function(a, b)
           {
               var result = whichFunction(a, b);
               return inverse ? -result : result;
           };
}

function initSorting(table)
{
    table.find('thead tr.headings th').each(
        function()
        {
            var th = $(this),
                columnClass = th.attr('class'),
                columnIndex = th.index(),
                inverse = false;

            th.find('.sort-indicator').each(
                function()
                {
                    var sortIndicatorControl = $(this);
                    sortIndicatorControl.addClass('unsorted');
                    sortIndicatorControl.click(
                        function()
                        {
                            var allSortIndicators = th.parent().find('.sort-indicator');
                            allSortIndicators.removeClass('sorted-up');
                            allSortIndicators.removeClass('sorted-down');
                            allSortIndicators.removeClass('unsorted');
                            allSortIndicators.addClass('unsorted');

                            table.find('tbody td').filter(
                                function() { return $(this).index() === columnIndex; }
                            )
                            .sortElements(
                                createCompareFunction(inverse, columnClass),
                                function() { return this.parentNode; }
                            );

                            sortIndicatorControl.removeClass('sorted-up');
                            sortIndicatorControl.removeClass('sorted-down');
                            sortIndicatorControl.removeClass('unsorted');

                            if (inverse)
                                sortIndicatorControl.addClass('sorted-down');
                            else
                                sortIndicatorControl.addClass('sorted-up');

                            inverse = !inverse;
                        }
                    );
                }
            );
        }
    );
}

/* filter functions */

function defaultFilter(bodyTd)
{
    return true;
}

function createRegexFilterFunction(filterControl)
{
    filterControl.css('color', '');

    var filterRegexStr = filterControl.val();
    if (filterRegexStr == '')
        return defaultFilter;

    try
    {
        var filterRegex = new RegExp(filterRegexStr);
    }
    catch (e)
    {
        filterControl.css('color', 'red');
        return defaultFilter;
    }

    return function(bodyTd)
           {
               var message = $.text([bodyTd]);
               return message.match(filterRegex) != null;
           };
}

function createSelectFilterFunction(filterControl)
{
    var selectedOption = filterControl.find('option:selected');
    if (selectedOption.index() == 0)
        return defaultFilter;

    var selection = selectedOption.val();
    return function(bodyTd) { return $.text([bodyTd]) == selection; };
}

function createFilterFunction(filterControl, columnClass)
{
    var whichFunction;

    if (columnClass == 'location' || columnClass == 'message')
        whichFunction = createRegexFilterFunction(filterControl);
    else if (columnClass == 'category' || columnClass == 'severity')
        whichFunction = createSelectFilterFunction(filterControl);

    return whichFunction;
}

function createCombinedFilterFunction(filterControls)
{
    var filterFunctions = [];

    filterControls.each(
        function()
        {
            var filterControl = $(this),
                footerTd = filterControl.parent(),
                columnClass = footerTd.attr('class');

            filterFunctions.push(
                {
                    'columnClass': columnClass,
                    'function': createFilterFunction(filterControl, columnClass)
                }
            );
        }
    );

    return function(bodyTr)
           {
               var result = true;
               for (var i = 0; i < filterFunctions.length; ++i)
               {
                   var bodyTd = bodyTr.find('td.' + filterFunctions[i].columnClass);
                   var r2 = filterFunctions[i].function(bodyTd);
                   if (!r2)
                   {
                       result = false;
                       break;
                   }
               }
               return result;
           };
}

function initFiltering(table)
{
    table.find('thead tr.filters th .filter').each(
        function()
        {
            var filterControl = $(this);
            filterControl.change(
                function()
                {
                    var filterControls = filterControl.parent().parent().find('.filter');
                    var combinedFilterFunction = createCombinedFilterFunction(filterControls);

                    table.find('tbody tr').each(
                        function()
                        {
                            var bodyTr = $(this);
                            if (combinedFilterFunction(bodyTr))
                                bodyTr.show();
                            else
                                bodyTr.hide();
                        }
                    );
                }
            );
        }
    );
}

function init()
{
    var table = $('table#errors');
    table.addClass('styled-table');

    initSorting(table);
    initFiltering(table);
}

$(document).ready(init);
