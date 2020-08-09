/** Runs a search on the content of the static pages. */
function search( text )
{
  // Get search library from select menu
  let searchLib = document.getElementById('searchlib').value;

  // Execute search
  const result = findResult( searchLib, text )

  //TEST
  console.log( searchLib );
  console.log( "result", result );

  // Prepare and display search window
  {
    // Set title
    document.getElementById('searchText').innerHTML = text;

    // Insert results
    let count = 0;
    let searchList = "";
    for( let i = 0; i < result.length; ++i )
    {
      // Get result object and score from the result
      // (score value differ in their meaning for different search libraries.)
      let resultObj;
      let score;
      switch( searchLib )
      {
        case 'lunr':
          resultObj = searchIndex[ result[i].ref ];
          score = result[i].score.toFixed(3);
          break;

        case 'fuse':
          resultObj = result[i].item;
          score = result[i].score;
          break;
      }

      // Insert result as list item
      {
        // Digest text: either brief of first part of content.
        let digest = ( resultObj.brief == "" ) ? resultObj.content : resultObj.brief;

        // Eventually shorten digest to 160 characters
        if( digest.length > 160 )
          digest = digest.slice(0, 160) + "…";

        // Place as list item.
        searchList += "<li><span class='score'>Score: " + score + "</span><a href='" + resultObj.url + "'><h2>" +
                      resultObj.title + "</h2></a><p>" + digest +
                      "</p></li>";
        count++;
      }
    }
    if( count == 0 )
      document.getElementById('searchItems').innerHTML = "<li>No search result found</li>";
    else
      document.getElementById('searchItems').innerHTML = searchList;

    // Display search window
    document.getElementById('searchResultBox').style.display = "block";
  }
}

// -----------------------------------------------------------------------------

/** Closes the search window */
function closeSearch()
{
  document.getElementById('searchInput').value = "";
  document.getElementById('searchResultBox').style.display = "none";
}

// -----------------------------------------------------------------------------

/** A singe search operation. Different external library modules can be used here. */
function findResult( searchLib, text )
{
  switch( searchLib )
  {
    case 'lunr':
    {
      var idx = lunr(function () {
        this.field('title', { boost: 10 } )
        this.field('brief', { boost: 2 } )
        this.field('content')

        for( let i = 0; i < searchIndex.length; ++i )
          this.add({
            title: searchIndex[i].title,
            content: searchIndex[i].content,
            id: i })
      })

      return idx.search( text + '~1' ); // ~1 for fuzzy search up to one exchange
    }

    case 'fuse':
    {
      const options = {
        isCaseSensitive: false,
        includeScore: true,
        shouldSort: true,
        findAllMatches: true,
        ignoreLocation: true,
        ignoreFieldNorm: true,
        keys: ['title', 'brief', 'content']
      }
      const fuse = new Fuse( searchIndex, options );
      return fuse.search( text );
    }
  }
}

