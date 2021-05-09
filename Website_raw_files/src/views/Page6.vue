<template>
  <div class="container">
    <div class="row">
      <h1>Embedded Software Kompilieren</h1>
      <div class="col-md">
      </div>
      <div class="col-md">
      <div class="rContainer" v-if="!build_started && !build_finished">
        <div v-if="!build_started">
          <button type="Button" class="btn btn-primary" @click="start_build()">Starte Kompilieren der Embedded Software</button>
          <div class="navbtn">
            <router-link to="/page5">
              <button @click="reset()" type="Button" class="btn btn-danger">Zurück</button>
            </router-link>
          </div>
        </div>

      </div>

        <div class="rContainer" v-if="build_started && !build_finished">
          <h2>Kompilierung läuft</h2>
          <div class="d-flex justify-content-center" >
            <div class="spinner-border m-3" role="status">
              <span class="sr-only"></span>
            </div>
          </div>
        </div>
      <div class="rContainer" v-if="build_finished">
        <div>
          <h2>Embedded Software erfolgreich kompiliert.</h2>
          <router-link to="/page5">
            <button @click="reset()" type="Button" class="btn btn-danger">Zurück</button>
          </router-link>
          <router-link to="/page7">
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
    start_build() {
      console.log("Model build started")
      this.build_started = true;
      let currentObj = this;
      this.axios.get("http://127.0.0.1:5000/do?key=build")
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
      this.axios.get("http://127.0.0.1:5000/check?key=build")
          .then(function (response) {
            if (response.data == "True") {
              currentObj.build_finished = true;
              clearInterval(currentObj.timer)
            }

          })
          .catch(function (error) {
          });

    },
    reset() {
      let currentObj = this;
      this.axios.get("http://127.0.0.1:5000/undo?key=build")
          .then(function (response) {
            currentObj.build_finished = false;
            currentObj.build_started = false;
          })
          .catch(function (error) {
          });

    }
  },
  name: "page6",
  data: function () {
    return {
      build_finished: false,
      build_started: false
    }
  }
}
</script>

<style>
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
