<template>
  <div class="container">
    <div class="row">
      <h1>Embedded Software flashen</h1>
      <div class="col-md">
      </div>
      <div class="col-md">
        <div class="rContainer" v-if="!flash_started && !flash_finished">
          <div v-if="!flash_started">
            <button type="Button" class="btn btn-primary" @click="start_flash()">Starte Flashen der Embedded-Software
            </button>
            <div class="navbtn">
              <router-link to="/page5">
                <button @click="reset()" type="Button" class="btn btn-danger">Zurück</button>
              </router-link>
            </div>
          </div>

        </div>

        <div class="rContainer" v-if="flash_started && !flash_finished">
          <h2>Flash läuft</h2>
          <div class="d-flex justify-content-center">
            <div class="spinner-border m-3" role="status">
              <span class="sr-only"></span>
            </div>
          </div>
        </div>
        <div class="rContainer" v-if="flash_finished">
          <div>
            <h2>Embedded Software erfolgreich geflashed.</h2>
            <router-link to="/page6">
              <button @click="reset()" type="Button" class="btn btn-danger">Zurück</button>
            </router-link>
            <router-link to="/page8">
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
    start_flash() {
      console.log("Model flash started")
      this.flash_started = true;
      let currentObj = this;
      this.axios.get("http://127.0.0.1:5000/do?key=flash")
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
      this.axios.get("http://127.0.0.1:5000/check?key=flash")
          .then(function (response) {
            if (response.data == "True") {
              currentObj.flash_finished = true;
              clearInterval(currentObj.timer)
            }

          })
          .catch(function (error) {
          });
    },
    reset() {
      let currentObj = this;

      currentObj.flash_finished = false;
      currentObj.flash_started = false;

    }
  },
  name: "page7",
  data: function () {
    return {
      flash_started: false,
      flash_finished: false
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
