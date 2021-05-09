<template>
  <div class="container">
    <div class="row">
      <h1>Klassenwahl</h1>
      <div class="col-md">
      </div>
      <div class="col-md">
        <div class="rContainer">
          <form>
            <div class="mb-3">
              <label for="input_klassen" class="form-label">Wie viele Klassen möchten Sie klassifizieren?</label>
              <input v-model="klassen_anzahl" type="number" class="form-control" id="input_klassen">
            </div>

            <div class="mb-3" v-for="n in parseInt(klassen_anzahl)">
              <label for="input_klassen" class="form-label">Geben Sie Klasse {{ n }} ein</label>
              <input v-model="klassen[n-1]" type="text" class="form-control" :id="'klasse'+n">
            </div>
            <form v-if="ds_checked">
              <div class="mb-3">
                <label for="num_images" class="form-label">Anzahl der Bilder pro Klasse</label>
                <input v-model="num_images" type="text" class="form-control" id="num_images">
              </div>
            </form>
            <div class="form-check">
              <input v-model="ds_checked" type="checkbox" class="form-check-input" id="exampleCheck1">
              <label class="form-check-label" for="exampleCheck1">Datensatz automatisch generieren?
                (Experimentell)</label>
            </div>

            <router-link to="/page2a">
              <button @click="start_image_gathering(klassen)" type="Button" class="btn btn-primary">Weiter</button>
            </router-link>

          </form>
        </div>
      </div>
        <div class="col-md">
        </div>
      </div>
    </div>
</template>
<script>
export default {
  name: "page1",
  data: function () {
    return {
      klassen_anzahl: 2,
      klassen: [],
      ds_checked: false,
      num_images: 20
    }
  },
  methods: {
    start_image_gathering(klassen) {
      if (this.ds_checked) {
        let currentObj = this;
        this.axios.post("http://127.0.0.1:5000/do?key=image_preparation&type=auto&num_images="+ this.num_images, {
          klassen: JSON.stringify(this.klassen)
        }).then(function (response) {
          currentObj.output = response.data;
        }).catch(function (error) {
          currentObj.output = error;
        });
      } else {
        let currentObj = this;
        this.axios.post("http://127.0.0.1:5000/do?key=image_preparation&type=manual", {
          klassen: JSON.stringify(this.klassen)
        }).then(function (response) {
          currentObj.output = response.data;
        }).catch(function (error) {
          currentObj.output = error;
        });
      }

    }
  }

}
</script>
