$(function(){
    var slides=$('.slider>li');
    var slideCount=0;
    var totalSlides=slides.length; //10
    var slideCash=[];

    (function preloader(){
        if(slideCount<totalSlides){
            slideCash[slideCount]=new Image();
            slideCash[slideCount].src=slides.eq(slideCount).find('img').attr('src');
            slideCash[slideCount].onload=function(){
                slideCount++;
                preloader();
            }
        }
        else{
            slideCount=0;
            SlideShow();
        }
    }());
    function SlideShow(){
        slides.eq(slideCount).fadeIn(1000).delay(3000).fadeOut(1000,function(){
            slideCount < totalSlides-1 ? slideCount ++ : slideCount = 0;
            SlideShow();
        });
    }

});