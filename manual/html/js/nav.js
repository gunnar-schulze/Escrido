window.addEventListener("load", function(event) {
  InitNav();
});

function InitNav()
{
  // Step 1: find the main <nav> element
  let mainNavElmt;
  {
    let foundMainNav = false;

    // Visit all <nav> elements of the page
    const navList = document.getElementsByTagName("NAV");
    for( let n = 0; n < navList.length; ++n )
    {
      const navElmt = navList[n];

      // Visit all <li> elements of the <nav> element
      const listItemList = navElmt.getElementsByTagName("LI");
      for( let li = 0; li < listItemList.length; ++li )
      {
        const listItemElmt = listItemList[li];

        // Search for "activepage" class
        if( listItemElmt.classList.contains( "activepage" ) )
        {
          // => Found main <nav> element.

          mainNavElmt = navElmt;
          foundMainNav = true;
          break;
        }
      }

      if( foundMainNav )
        break;
    }
  }

  // Make all <h2>...<h5> elements of the <nav> element clickable
  let headerList = mainNavElmt.querySelectorAll('h2,h3,h4,h5');
  for( let i = 0; i < headerList.length; ++i )
  {
    // Make head of class "expanded" to style it
    headerList[i].classList.add( "expanded" );

    // Contract elements that are not in class "activepage"
    const parentLI = headerList[i].parentElement;
    if( !parentLI.classList.contains( "activepage" ) )
    {
      parentLI.querySelector( "ul" ).classList.toggle( "invisible" );
      headerList[i].classList.toggle( "contracted" );
    }

    // Add event listener to expand/contract section
    headerList[i].addEventListener( "click", function()
    {
      this.parentElement.querySelector( "ul" ).classList.toggle( "invisible" );
      this.classList.toggle( "contracted" );
    });
  }
}
