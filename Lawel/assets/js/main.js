(function($) {
   "use strict";

    $(document).ready(function () {

        /**-------------------------------------------
         *  Navbar fix
         * -----------------------------------------*/
        $(document).on('click', '.navbar-area .navbar-nav li.menu-item-has-children>a', function (e) {
            e.preventDefault();
        })
       
        /*-------------------------------------
            menu
        -------------------------------------*/
        $('.navbar-area .menu').on('click', function() {
            $(this).toggleClass('open');
            $('.navbar-area .navbar-collapse').toggleClass('sopen');
        });
    
        // mobile menu
        if ($(window).width() < 992) {
            $(".in-mobile").clone().appendTo(".sidebar-inner");
            $(".in-mobile ul li.menu-item-has-children").append('<i class="fas fa-chevron-right"></i>');
            $('<i class="fas fa-chevron-right"></i>').insertAfter("");

            $(".menu-item-has-children a").on('click', function(e) {
                // e.preventDefault();

                $(this).siblings('.sub-menu').animate({
                    height: "toggle"
                }, 300);
            });
        }

        var menutoggle = $('.menu-toggle');
        var mainmenu = $('.navbar-nav');
        
        menutoggle.on('click', function() {
            if (menutoggle.hasClass('is-active')) {
                mainmenu.removeClass('menu-open');
            } else {
                mainmenu.addClass('menu-open');
            }
        });

        // off canvas menu
        $("#side-menu-open").click(function(){
            $("#side-navbar").animate({
                left: '0'     
            });
        });
        $("#side-menu-close").click(function(){
            $("#side-navbar").animate({
                left: '-300px'          
            }); 
        });

        /*--------------------------------------------------
            select onput
        ---------------------------------------------------*/
        if ($('.single-select').length){
            $('.single-select').niceSelect();
        }


        /* -----------------------------------------------------
            Variables
        ----------------------------------------------------- */
        var leftArrow = '<i class="fa fa-arrow-left"></i>';
        var leftAngle = '<i class="fa fa-angle-left"></i>';
        var rightArrow = '<i class="fa fa-arrow-right"></i>';
        var rightAngle = '<i class="fa fa-angle-right"></i>';
        var backButton = '<button class="slide-arrow prev-arrow"><i class="fa fa-angle-left"></i></button>';
        var nextButton = '<button class="slide-arrow next-arrow"><i class="fa fa-angle-right"></i></button>';

        /**banner-slider**/
        $('.banner-slider').owlCarousel({
            loop:true,
            nav:true,
            dots: true,
            items: 1,
            smartSpeed:1500,
            navText: [leftAngle,rightAngle],
        })

        /**banner-slider-2**/
        $('.banner-slider-2').owlCarousel({
            loop:true,
            nav:true,
            dots: true,
            margin: 20,
            items: 3,
            smartSpeed:1500,
            navText: [leftAngle,rightAngle],
            responsive:{
                0:{
                    items:1
                },
                576:{
                    items:2
                },
                789:{
                    items:3
                },
            }
        })

        /**instagram-slider**/
        $('.instagram-slider').owlCarousel({
            loop:true,
            margin: 0,
            nav:true,
            dots: true,
            smartSpeed:1500,
            navText: [leftAngle,rightAngle],
            responsive:{
                0:{
                    items:2
                },
                767:{
                    items:4
                },
                1300:{
                    items:6
                }
            }
        })

        /**---------------------------------------
          Progress BAR
        ----------------------------------------*/
        function td_Progress() {
            var progress = $('.progress-rate');
            var len = progress.length;
             for (var i = 0; i < len; i++) {
                var progressId = '#' + progress[i].id;
                var dataValue = $(progressId).attr('data-value');
                $(progressId).css({'width':dataValue+'%'});
             }
         }
         var progressRateClass = $('.progress-item');
          if ((progressRateClass).length) {
             td_Progress();
         }
         $('.counting').each(function() {
             var $this = $(this),
             countTo = $this.attr('data-count');
           
             $({ countNum: $this.text()}).animate({
                 countNum: countTo
             },
 
             {
                 duration: 2000,
                 easing:'linear',
                 step: function() {
                     $this.text(Math.floor(this.countNum));
                 },
                 complete: function() {
                     $this.text(this.countNum);
                 }
             });  
         });
          

        /* -------------------------------------------------------------
         fact counter
         ------------------------------------------------------------- */
        $('.counter').counterUp({
            delay: 15,
            time: 2000
        });

        /*------------------------------------------------
            Magnific JS
        ------------------------------------------------*/
        $('.play-btn').magnificPopup({
            type: 'iframe',
            removalDelay: 260,
            mainClass: 'mfp-zoom-in',
        });
        $.extend(true, $.magnificPopup.defaults, {
            iframe: {
                patterns: {
                    youtube: {
                        index: 'youtube.com/',
                        id: 'v=',
                        src: 'https://www.youtube.com/embed/Wimkqo8gDZ0'
                    }
                }
            }
        });

        /*--------------------------------------------
            Search Popup
        ---------------------------------------------*/
        var bodyOvrelay =  $('#body-overlay');
        var searchPopup = $('#search-popup');

        $(document).on('click','#body-overlay',function(e){
            e.preventDefault();
        bodyOvrelay.removeClass('active');
            searchPopup.removeClass('active');
        });
        $(document).on('click','.search',function(e){
            e.preventDefault();
            searchPopup.addClass('active');
        bodyOvrelay.addClass('active');
        });

        /**featured-accordion**/
        $('.accordion-item').on('click',function(e){
            $('.accordion-item').removeClass('active');
            $(this).toggleClass('active');
        });

        /*------------------
           back to top
        ------------------*/
        $(document).on('click', '.back-to-top', function () {
            $("html,body").animate({
                scrollTop: 0
            }, 200);
        });

    });

    $(window).on("scroll", function() {
        /*---------------------------------------
        sticky menu activation && Sticky Icon Bar
        -----------------------------------------*/

        var mainMenuTop = $(".navbar-area");
        if ($(window).scrollTop() >= 1) {
            mainMenuTop.addClass('navbar-area-fixed');
        }
        else {
            mainMenuTop.removeClass('navbar-area-fixed');
        }
        
        var ScrollTop = $('.back-to-top');
        if ($(window).scrollTop() > 1000) {
            ScrollTop.fadeIn(1000);
        } else {
            ScrollTop.fadeOut(1000);
        }
    });


    $(window).on('load', function () {

        /*-----------------
            preloader
        ------------------*/
        var preLoder = $("#preloader");
        preLoder.fadeOut(0);

        /*-----------------
            back to top
        ------------------*/
        var backtoTop = $('.back-to-top')
        backtoTop.fadeOut();

        /*---------------------
            Cancel Preloader
        ----------------------*/
        $(document).on('click', '.cancel-preloader a', function (e) {
            e.preventDefault();
            $("#preloader").fadeOut(500);
        });

    });


})(jQuery);