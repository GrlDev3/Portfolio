window.addEventListener('load', function() {
    document.forms['signup'].onsubmit = function(){
        return validateForm();
    }
});

function validateForm () {
    var user = document.forms['signup']['username'];
    var email = document.forms['signup']['email'];
    var pass = document.forms['signup']['password'];

    var b_user = false;
    var b_email = false;
    var b_pass = false;

    var text = "";
    var rex = /^\w+([\.-]?\w+)*@\w+([\.-]?\w+)*(\.\w{2,3})+$/;

    // ellenorzi, hogy ures-e a mezo es megfelelo hosszusagu-e
    if(user.value.trim() == null || user.value.trim() == ''){
        user.style.backgroundColor = "#f00";
        text += "You must enter an username!<br />";
        b_user = false;
    }else if(user.value.trim().length < 4 || user.value.trim().length > 30){
        user.style.backgroundColor = "#f00";
        text += "The username must be between 4 and 30 characters.<br />";
        b_user = false;
    }else{
        user.style.backgroundColor = "#0f0";
        text += "";
        b_user = true;
    }

    // ellenorzi, hogy ures-e a mezo, megfelelo hosszusagu-e // es van-e benne szokoz - ezt nem egyelore
    if( email.value.trim() == null || email.value.trim() == ''){
        email.style.backgroundColor = "#f00";
        text += "You must enter an email address!<br />";
        b_email = false;
    }else if(email.value.trim().length < 4 || email.value.trim().length > 35){
        email.style.backgroundColor = "#f00";
        text += "The email must be between 4 and 35 characters.<br />";
        b_email = false;
    }else if(!rex.test(email.value)){
        email.style.backgroundColor = "#f00";
        text += "The email's format is incorrect.<br />";
        b_email = false;
    }
    else{
        email.style.backgroundColor = "#0f0";
        text += "";
        b_email = true;
    }

    // ellenorzi, hogy ures-e a mezo es megfelelo hosszusagu-e
    if( pass.value.trim() == null || pass.value.trim() == ''){
        pass.style.backgroundColor = "#f00";
        text += "You must enter the password!<br />";
        b_pass = false;
    }else if(pass.value.trim().length < 4 || pass.value.trim().length > 20){
        pass.style.backgroundColor = "#f00";
        text += "The password must be between 4 and 20 characters.<br />";
        b_pass = false;
    }else{
        pass.style.backgroundColor = "#0f0";
        text += "";
        b_pass = true;
    }

    if(text != ""){
        document.getElementById('error').innerHTML = text;
    }

    return (b_pass && b_user && b_email);

}