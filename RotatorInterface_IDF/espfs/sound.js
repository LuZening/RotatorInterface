var soundSuccess = document.createElement("audio")
soundSuccess.src = "success.mp3"
function play_sound_success()
{
    soundSuccess.play();
}

var soundFailure = document.createElement("audio")
soundFailure.src = "failure.mp3"
function play_sound_failure()
{
    soundFailure.play();
}