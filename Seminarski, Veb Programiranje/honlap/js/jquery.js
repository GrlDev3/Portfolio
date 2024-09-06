/* jQuery */
var jq = $.noConflict();

jq(document).ready(function(){
    //ha a termek nevere kattintva, megjelenithetjuk/eltuntethetjuk a leirasat
    jq(".show").click(function(){
        jq(".description").toggle("slow");
    });

    //az adott lehetoseg nevere kattintva lecsusztathatjuk/felcsusztathatjuk a bovebb leirast, akar egy drop down listat
    jq("#new").click(function(){
        jq("#descriptionnew").slideToggle("slow");
    });
    jq("#list").click(function(){
        jq("#descriptionlist").slideToggle("slow");
    });
    jq("#update").click(function(){
        jq("#descriptionupdate").slideToggle("slow");
    });
    jq("#delete").click(function(){
        jq("#descriptiondelete").slideToggle("slow");
    });

});
