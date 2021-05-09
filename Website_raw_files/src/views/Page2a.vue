<template>
  <div class="container">
    <div class="row">
      <h1>Trainingsdatensatz Anpassen</h1>
      <div class="col-md">
      </div>
      <div class="col-md">

        <div class="all_images" v-if="!isFetching && finished_gathering">
          <div v-for="(klasse, index) in klassen" :key="klasse" class="all_class_images">
            <h1>{{ klasse }}</h1>
            <hr/>
            <div>
              <rImage v-for="image in images[index]" :key="image" :path="image"></rImage>
            </div>

            <form>
              <label>Files
                <input type="file" id="files" ref="files" multiple v-on:change="handleFilesUpload($event)"/>
              </label>
              <button v-on:click="submitFiles(klasse)">Submit</button>
            </form>
          </div>
        </div>
        <div>
          <router-link to="/page1">
            <button type="Button" class="btn btn-danger">Zurück</button>
          </router-link>
          <router-link to="/page3">
            <button type="Button" class="btn btn-primary">Weiter</button>
          </router-link>
        </div>


        <div class="rContainer" v-if="!finished_gathering">
          <div class="d-flex justify-content-center">
            <div class="spinner-border m-3" role="status">
              <span class="sr-only"></span>
            </div>
          </div>
        </div>


      </div>
      <div class="col-md">
      </div>
    </div>
  </div>
</template>
<script>
import rImage from '@/components/rImage.vue';

export default {
  created: function () {
    this.isFetching = true;
    this.timer = setInterval(() => {
      this.check_finished()
    }, 2000)
    this.get_classes();
  },
  name: "page2a",
  methods: {
    check_finished() {
      console.log("check")
      let currentObj = this;
      this.axios.get("http://127.0.0.1:5000/check?key=image_preparation")
          .then(function (response) {
            if (response.data == "True") {
              currentObj.finished_gathering = true;
              currentObj.refresh();
              clearInterval(currentObj.timer)
            }

          })
          .catch(function (error) {
          });

    },
    submitFiles(klasse) {
      let currentObj = this;
      let formData = new FormData();

      for (var i = 0; i < this.files.length; i++) {

        let file = this.files[i];
        console.log("file" + file)
        formData.append('files', file);
      }


      this.axios.post('http://127.0.0.1:5000/upload?rclass=' + klasse,
          formData,
          {
            headers: {
              'Content-Type': 'multipart/form-data'
            }
          }
      ).then(function () {
        console.log('SUCCESS!!');
        setTimeout(() => {
          currentObj.refresh();
        }, 10000);

      })
          .catch(function () {
            console.log('FAILURE!!');
          });
    },
    handleFilesUpload(event) {
      console.log("event" + event.target.files[0])
      this.files = event.target.files
      console.log(this.files[0]);
    },
    refresh() {
      console.log("REFRESH")
      this.isFetching = true;
      this.get_classes();
    },
    get_images(rclass, index) {
      let theURL = "http://127.0.0.1:5000/check?key=images&rclass=" + rclass
      let currentObj = this;
      this.axios.get(theURL).then(function (response) {
        currentObj.images[index] = response.data;
      }).catch(function (error) {
        currentObj.output = error;
      });
    },
    get_classes() {
      let theURL = "http://127.0.0.1:5000/check?key=classes"
      let currentObj = this;
      this.axios.get(theURL).then(function (response) {
        currentObj.klassen = response.data;
      }).then(function () {
        setTimeout(() => {
          currentObj.klassen.forEach(function (item, index) {

            currentObj.get_images(item, index);
          })
        }, 500);

      }).then(function () {
        setTimeout(() => {
          currentObj.isFetching = false
        }, 500);
      }).catch(function (error) {
        currentObj.output = error;
      });
    }
  },
  components: {
    rImage
  },
  data: function () {
    return {
      files: "",
      images: [],
      klassen: [],
      isFetching: true,
      finished_gathering: false
    }
  }
}
</script>

<style>
.all_class_images {
  height: 100%;
  width: 100%;
  display: inline-block;
  border-style: solid;
  border-radius: 10px;
  border-width: 2px;
  box-shadow: 5px 10px 18px #888888;
  margin: 5px;
  padding: 2px
}
</style>
