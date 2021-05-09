<template>
  <div class="container">
    <div class="row">
      <h1>Modell erstellen</h1>
      <div class="col-md">
      </div>
      <div class="col-md">

        <div class="rContainer" v-if="!training_finished && !training_started">
          <div class="navbtn">
            <button type="Button" class="btn btn-primary" @click="start_training()">Modell Training starten</button>
          </div>
          <div class="navbtn">
            <router-link to="/page3">
              <button @click="reset()" type="Button" class="btn btn-danger">Zurück</button>
            </router-link>
          </div>
        </div>
        <div class="rContainer" v-if="!training_finished && training_started">

          <h2>Bitte warten Sie während Ihr Modell erstellt wird.</h2>

          <div class="d-flex justify-content-center">
            <div class="spinner-border m-3" role="status">
              <span class="sr-only"></span>
            </div>
          </div>

          <div class="d-flex justify-content-center" v-if="training_finished">
            <h3>Ihr Modell wurde erstellt. Bitte klicken Sie auf weiter.</h3>
          </div>


        </div>

        <div class="rContainer" v-if="training_finished">

          <h2>Ihr Modell wurde erstellt. Bitte klicken Sie weiter.</h2>

          <div>
            <router-link to="/page3">
              <button @click="reset()" type="Button" class="btn btn-danger">Zurück</button>
            </router-link>
            <router-link to="/page5">
              <button type="Button" class="btn btn-primary">Weiter</button>
            </router-link>
          </div>


        </div>


      </div>
      <div class="col-md">
      </div>
    </div>
  </div>
</template>
<script>
export default {
  mounted: function () {

  },
  methods: {
    start_training() {
      console.log("Model Training started")
      this.training_started = true;
      let currentObj = this;
      this.axios.get("http://127.0.0.1:5000/do?key=training")
          .then(function (response) {
            currentObj.timer = setInterval(() => {
              currentObj.check_finished()
            }, 2000)
          })
          .catch(function (error) {
          });
    },
    check_finished() {
      console.log("check")
      let currentObj = this;
      this.axios.get("http://127.0.0.1:5000/check?key=training")
          .then(function (response) {
            if (response.data == "True") {
              currentObj.training_finished = true;
              clearInterval(currentObj.timer)
            }

          })
          .catch(function (error) {
          });

    },
    reset() {
      let currentObj = this;
      this.axios.get("http://127.0.0.1:5000/undo?key=training")
          .then(function (response) {
            currentObj.training_finished = false;
            currentObj.training_started = false;
          })
          .catch(function (error) {
          });

    }
  },
  name: "page4",
  data: function () {
    return {
      training_started: false,
      training_finished: false
    }
  }
}
</script>

<style>
.navbtn {
  margin: 10px;
}

.rContainer {
  height: 100%;
  width: 100%;
  display: inline-block;
  border-style: solid;
  border-radius: 10px;
  border-width: 2px;
  box-shadow: 5px 10px 18px #888888;
  margin: 5px;
  padding: 20px 20px;
}
</style>
